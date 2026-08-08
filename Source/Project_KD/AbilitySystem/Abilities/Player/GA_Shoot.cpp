// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_Shoot.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Shoot::UGA_Shoot()
{
	bRetriggerInstancedAbility = false; 
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 기능 : 조준 자세 위에 발사 몽타주 1회
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!ShootMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, ShootMontage, MontagePlayRate, NAME_None, true, 1.0f);

	if (!Task)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Task->OnCompleted.AddDynamic(this, &UGA_Shoot::OnMontageFinished);
	Task->OnInterrupted.AddDynamic(this, &UGA_Shoot::OnMontageFinished);
	Task->OnCancelled.AddDynamic(this, &UGA_Shoot::OnMontageFinished);
	Task->ReadyForActivation();

	// 몽타주 콜백 유실 시 EndAbility
	StartSafetyTimer(ShootMontage->GetPlayLength(), MontagePlayRate);
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Shoot::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
