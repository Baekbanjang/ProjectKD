// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Enemy/GA_EnemyParry.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "KDGameplayTags.h"

UGA_EnemyParry::UGA_EnemyParry()
{
	bRetriggerInstancedAbility = false;
	ParryClashCueTag = GameplayTags::GameplayCue_Combat_ParryClash;
}

void UGA_EnemyParry::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// InstancedPerActor 잔류 방지 — 매 활성화 시 명시 리셋.
	ActiveParryHandle = FActiveGameplayEffectHandle();

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!IsValid(ASC) || !ParryMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// State.Combat.Parrying 부여 GE(Infinite) — OnCleanup에서 제거. AS_Combat이 이 태그로 데미지를 막는다.
	ActiveParryHandle = ApplySelfEffect(ParryGE);

	// 가드 윈도우 동안 막아낸 히트마다 클래시 GC를 띄우기 위해 OnlyTriggerOnce=false. GA 종료 시 자동 정리.
	UAbilityTask_WaitGameplayEvent* ParryHitListener = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Combat_ParrySuccess, nullptr, false, true);
	ParryHitListener->EventReceived.AddDynamic(this, &UGA_EnemyParry::OnParrySuccess);
	ParryHitListener->ReadyForActivation();

	// 패링 몽타주 — 끝나거나 끊기면 GA 종료 = 가드 윈도우 종료(몽타주 길이가 곧 가드 시간).
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ParryMontage, MontagePlayRate, NAME_None, true, 1.0f);
	if (!Task)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	Task->OnCompleted.AddDynamic(this, &UGA_EnemyParry::OnParryMontageEnded);
	Task->OnInterrupted.AddDynamic(this, &UGA_EnemyParry::OnParryMontageEnded);
	Task->OnCancelled.AddDynamic(this, &UGA_EnemyParry::OnParryMontageEnded);
	Task->ReadyForActivation();

	StartSafetyTimer(ParryMontage->GetPlayLength(), MontagePlayRate);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_EnemyParry::OnParryMontageEnded()
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyParry::OnParrySuccess(FGameplayEventData Payload)
{
	if (!ParryClashCueTag.IsValid()) return;
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		// ContextHandle의 공격자 HitResult(ImpactPoint)가 GC 위치로 들어간다 — 막아낸 접촉점에 스파크.
		ASC->ExecuteGameplayCue(ParryClashCueTag, Payload.ContextHandle);
	}

	// 패링 성공 시 Success 섹션으로 점프 — 이미 그 섹션이면 재점프 금지(OnlyTriggerOnce=false라 히트마다 호출됨).
	if (!ParryMontage) return;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) return;

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance) return;

	if (AnimInstance->Montage_GetCurrentSection(ParryMontage) != ParrySuccessSection)
		AnimInstance->Montage_JumpToSection(ParrySuccessSection, ParryMontage);
}

void UGA_EnemyParry::OnCleanup(bool bWasCancelled)
{
	if (ActiveParryHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(ActiveParryHandle);
		}
		ActiveParryHandle = FActiveGameplayEffectHandle();
	}
}
