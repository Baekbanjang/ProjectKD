// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_Parry.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UKDGameplayAbility_Parry::UKDGameplayAbility_Parry()
{
	bRetriggerInstancedAbility = false;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKDGameplayAbility_Parry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 기능 : 방어 GE 2개 부여 후 가드 진입 모션 재생
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// InstancedPerActor 잔류 방지 — 활성화마다 핸들 리셋
	ActivePerfectWindowHandle = FActiveGameplayEffectHandle();
	ActiveBlockHandle = FActiveGameplayEffectHandle();

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!IsValid(ASC))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 홀드 방어 GE — Infinite | 제거 = OnCleanup
	ActiveBlockHandle = ApplySelfEffect(BlockGE);

	// 퍼펙트 패링 창 GE — 0.15s 후 자동 종료
	ActivePerfectWindowHandle = ApplySelfEffect(PerfectParryWindowGE);

	// 막힌 히트 수신 — 홀드 중 반복 수신이라 OnlyTriggerOnce = false
	UAbilityTask_WaitGameplayEvent* HitListener = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Combat_HitReact, nullptr, false, true);
	HitListener->EventReceived.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockHitReceived);
	HitListener->ReadyForActivation();

	// 가드 진입 모션 — 종료 후 자세 = 가드 로코 PSD
	if (BlockStartMontage)
	{
		UAbilityTask_PlayMontageAndWait* StartTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BlockStartMontage, MontagePlayRate, NAME_None, true, 1.0f);
		if (StartTask)
		{
			StartTask->OnInterrupted.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartInterrupted);
			StartTask->OnCancelled.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartInterrupted);
			StartTask->ReadyForActivation();
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UKDGameplayAbility_Parry::OnCleanup(bool bWasCancelled)
{
	// 기능 : 활성 방어 GE 2개 제거
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// 퍼펙트 패링 창 GE 제거
	if (ActivePerfectWindowHandle.IsValid())
	{
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActivePerfectWindowHandle);
		}
		ActivePerfectWindowHandle = FActiveGameplayEffectHandle();
	}

	// 홀드 방어 GE 제거
	if (ActiveBlockHandle.IsValid())
	{
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActiveBlockHandle);
		}
		ActiveBlockHandle = FActiveGameplayEffectHandle();
	}
	// BlockEndMontage 재생 X — 캔슬 시점에 GA 즉시 종료
}

void UKDGameplayAbility_Parry::OnBlockStartInterrupted()
{
	// 기능 : 가드 진입 모션 중단 시 GA 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKDGameplayAbility_Parry::OnBlockHitReceived(FGameplayEventData Payload)
{
	// 기능 : 막힌 히트에만 플린치 모션 재생 — 종료 후 복귀 = 가드 로코 PSD
	// 무방비 = magnitude 0.0 — GA_HitReact 담당
	if (Payload.EventMagnitude < 0.5f) return;
	if (!BlockHitMontage) return;

	UAbilityTask_PlayMontageAndWait* HitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, BlockHitMontage, MontagePlayRate, NAME_None, true, 1.0f);
	if (HitTask)
	{
		HitTask->ReadyForActivation();
	}
}

