// Fill out your copyright notice in the Description page of Project Settings.


#include "KDGameplayAbility_SkillCharge.h"

#include "KDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"      
#include "Animation/AnimMontage.h"       
#include "Engine/World.h"                
#include "TimerManager.h"                

void UKDGameplayAbility_SkillCharge::OnActivated()
{
	// 기능 : 차지 시작 — 안전 타이머 해제 후 정지 예약 + 릴리즈 대기
	Super::OnActivated();
	
	bChargeReleased = false;
	ChargeStartTime = 0.f;
	
	UWorld* World = GetWorld();
	if (!World || !IsValid(AttackMontage))
	{
		return;
	}
	
	// 루프가 몽타주 길이를 넘기므로 부모의 강제 종료 타이머 해제
	ClearSafetyTimer();
	ChargeStartTime = World->GetTimeSeconds();
	
	const float Rate = FMath::Max(GetEffectiveMontagePlayRate(), 0.1f);
	// 진입 섹션(0번) 끝에서 정지
	const float FreezeDelay = AttackMontage->GetSectionLength(0) / Rate;
	World->GetTimerManager().SetTimer(FreezeTimerHandle, this,
		&UKDGameplayAbility_SkillCharge::FreezeAtChargeStart, FreezeDelay, false);
	
	// 릴리즈 신호 유실 대비 상한
	World->GetTimerManager().SetTimer(MaxHoldTimerHandle, this,
		&UKDGameplayAbility_SkillCharge::ReleaseCharge, MaxHoldTime, false);
	UAbilityTask_WaitGameplayEvent* ReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Skill_HoldRelease, nullptr, true, true);
	ReleaseTask->EventReceived.AddDynamic(this, &UKDGameplayAbility_SkillCharge::OnHoldReleased);
	ReleaseTask->ReadyForActivation();
}

void UKDGameplayAbility_SkillCharge::OnCleanup(bool bWasCancelled)
{
	// 기능 : 타이머 정리 
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FreezeTimerHandle);
		World->GetTimerManager().ClearTimer(MaxHoldTimerHandle);
	}
	Super::OnCleanup(bWasCancelled);
}

void UKDGameplayAbility_SkillCharge::FreezeAtChargeStart()
{
	// 기능 : 몽타주 정지 — 정지 중 루트모션 델타 0
	if (bChargeReleased)
	{
		return;
	}
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	UAnimInstance* Anim = Info ? Info->GetAnimInstance() : nullptr;
	if (Anim && IsValid(AttackMontage))
	{
		Anim->Montage_Pause(AttackMontage);
	}
}

void UKDGameplayAbility_SkillCharge::ReleaseCharge()
{
	// 기능 : 홀드 시간으로 차지 단계 결정 -> 마무리 링크 교체 -> 재개
	if (bChargeReleased)
	{
		return;
	}
	bChargeReleased = true;
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FreezeTimerHandle);
		World->GetTimerManager().ClearTimer(MaxHoldTimerHandle);
	}
	
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	UAnimInstance* Anim = Info ? Info->GetAnimInstance() : nullptr;
	if (!Anim || !IsValid(AttackMontage))
	{
		return;
	}
	
	// 홀드 시간 — 타이머와 같은 게임 시계
	const float HoldTime = World ? (World->GetTimeSeconds() - ChargeStartTime) : 0.f;
	
	// 조건을 만족하는 마지막 단계 선택
	const FKDChargeStep* Step = nullptr;
	for (const FKDChargeStep& Candidate : ChargeSteps)
	{
		// 홀드 시간이 넘으면 다음 단계
		if (HoldTime >= Candidate.MinHoldTime)
		{
			Step = &Candidate;
		}
	}
	
	if (Step && Step->SectionName != NAME_None)
	{
		Anim->Montage_SetNextSection(Step->SectionName, EndSectionName, AttackMontage);
	}
	Anim->Montage_Resume(AttackMontage);
	
	// 남은 재생분 기준 안전 타이머 복구
	StartSafetyTimer(AttackMontage->GetPlayLength(), FMath::Max(GetEffectiveMontagePlayRate(), 0.1f));
}

void UKDGameplayAbility_SkillCharge::OnHoldReleased(FGameplayEventData Payload)
{
	ReleaseCharge();
}
