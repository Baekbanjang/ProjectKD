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
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// InstancedPerActor 잔류 방지 — 매 활성화 시 명시 리셋.
	ActivePerfectWindowHandle = FActiveGameplayEffectHandle();
	ActiveBlockHandle = FActiveGameplayEffectHandle();

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!IsValid(ASC))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 홀드 방어 GE 적용 (Infinite, 버튼 누르는 동안 50% 감소). OnCleanup에서 제거.
	ActiveBlockHandle = ApplySelfEffect(BlockGE);

	// Perfect Parry 윈도우 GE 적용 (0.15s 후 자동 종료, State.Combat.PerfectParryReady 부여).
	ActivePerfectWindowHandle = ApplySelfEffect(PerfectParryWindowGE);

	// 홀드 동안 막힌 히트를 계속 받기 위해 OnlyTriggerOnce=false. GA 캔슬 시 자동 정리.
	UAbilityTask_WaitGameplayEvent* HitListener = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Combat_HitReact, nullptr, false, true);
	HitListener->EventReceived.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockHitReceived);
	HitListener->ReadyForActivation();

	// BlockStart 재생 -> 콜백에서 Loop로 전환.
	if (BlockStartMontage)
	{
		UAbilityTask_PlayMontageAndWait* StartTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, BlockStartMontage, MontagePlayRate, NAME_None, true, 1.0f);
		if (StartTask)
		{
			StartTask->OnCompleted.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartCompleted);
			StartTask->OnBlendOut.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartCompleted);
			StartTask->OnInterrupted.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartInterrupted);
			StartTask->OnCancelled.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockStartInterrupted);
			StartTask->ReadyForActivation();
		}
	}
	else
	{
		// BlockStart 없으면 바로 Loop로.
		OnBlockStartCompleted();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UKDGameplayAbility_Parry::OnCleanup(bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// 퍼펙트 윈도우 GE 제거
	if (ActivePerfectWindowHandle.IsValid())
	{
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActivePerfectWindowHandle);
		}
		ActivePerfectWindowHandle = FActiveGameplayEffectHandle();
	}

	// 홀드 방어 GE(Infinite) 제거 
	if (ActiveBlockHandle.IsValid())
	{
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActiveBlockHandle);
		}
		ActiveBlockHandle = FActiveGameplayEffectHandle();
	}
	// BlockEndMontage는 캔슬 시점에 GA가 즉시 종료라 재생 X. M2에 별도 처리 검토
}

void UKDGameplayAbility_Parry::OnBlockStartCompleted()
{
	PlayBlockLoop();
}

void UKDGameplayAbility_Parry::OnBlockStartInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKDGameplayAbility_Parry::OnBlockHitReceived(FGameplayEventData Payload)
{
	// 무방비(뒤/옆, magnitude 0.0)는 GA_HitReact가 처리 -> 블록 주인은 막힌 히트만 반응.
	if (Payload.EventMagnitude < 0.5f) return;
	if (!BlockHitMontage) return;

	UAbilityTask_PlayMontageAndWait* HitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, BlockHitMontage, MontagePlayRate, NAME_None, true, 1.0f);
	if (HitTask)
	{
		// Completed/BlendOut만 -> 루프 복귀. Interrupted는 다음 플린치가 담당(이중 복귀 방지).
		HitTask->OnCompleted.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockHitMontageEnded);
		HitTask->OnBlendOut.AddDynamic(this, &UKDGameplayAbility_Parry::OnBlockHitMontageEnded);
		HitTask->ReadyForActivation();
	}
}

void UKDGameplayAbility_Parry::OnBlockHitMontageEnded()
{
	PlayBlockLoop();
}

void UKDGameplayAbility_Parry::PlayBlockLoop()
{
	// BlockStart 종료 시 BlockLoop로 전환.
	if (!BlockLoopMontage) return;

	UAbilityTask_PlayMontageAndWait* LoopTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, BlockLoopMontage, MontagePlayRate, NAME_None, true, 1.0f);
	if (LoopTask)
	{
		// BlockLoop는 Montage 자체가 Loop=true라 종료 X. GA 캔슬 시 자동 정리.
		LoopTask->ReadyForActivation();
	}
}
