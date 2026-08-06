// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_AimMode.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_AimMode::UGA_AimMode()
{
	bRetriggerInstancedAbility = false; // 어빌리티 재시작 여부
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_AimMode::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 기능 : 조준 태그 유지하면서 진입 몽타주 1회 재생
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// 비용 | 쿨타임 충족 X -> 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	// 진입 몽타주 = 선택 사항
	if (AimStartMontage)
	{
		UAbilityTask_PlayMontageAndWait* StartTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AimStartMontage, MontagePlayRate, NAME_None, true, 1.0f);
		if (StartTask)
		{
			StartTask->ReadyForActivation();   // 태스크 시작 | 종료 콜백 X
		}
	}
	// 안전 타이머 X — 홀드 GA
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
