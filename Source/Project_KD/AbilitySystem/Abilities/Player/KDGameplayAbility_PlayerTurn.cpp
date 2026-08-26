// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerTurn.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "KDGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKDGameplayAbility_PlayerTurn::UKDGameplayAbility_PlayerTurn()
{
	bRetriggerInstancedAbility = true;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKDGameplayAbility_PlayerTurn::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// InstancedPerActor 잔류 방지 — 매 활성화 시 명시 리셋.
	bRestoreOrientRotation = false;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!IsValid(ASC))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* TurnChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* MoveComp = TurnChar ? TurnChar->GetCharacterMovement() : nullptr;
	if (!MoveComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 입력 방향 -> 메시 facing 부호있는 각도. +면 우(R)=[0], -면 좌(L)=[1].
	const FVector InputVec = MoveComp->GetLastInputVector();
	const float Angle = InputVec.IsNearlyZero()
		? 0.f
		: FMath::FindDeltaAngleDegrees(TurnChar->GetActorRotation().Yaw, InputVec.Rotation().Yaw);
	const int32 Idx = (Angle >= 0.f) ? 0 : 1;
	UAnimMontage* SelectedMontage = TurnMontages.IsValidIndex(Idx) ? TurnMontages[Idx].Get() : nullptr;

	if (!IsValid(SelectedMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 회전 권한을 GA로 — orient-to-movement가 켜져 있으면 끄고, OnCleanup에서 복원.
	if (MoveComp->bOrientRotationToMovement)
	{
		MoveComp->bOrientRotationToMovement = false;
		bRestoreOrientRotation = true;
	}

	// Tick 재트리거 차단용 상태 태그.
	ASC->AddLooseGameplayTag(GameplayTags::State_Movement_Turning);

	UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, SelectedMontage, MontagePlayRate, NAME_None, true, 1.0f);

	PlayTask->OnCompleted.AddDynamic(this, &UKDGameplayAbility_PlayerTurn::OnMontageCompleted);
	PlayTask->OnInterrupted.AddDynamic(this, &UKDGameplayAbility_PlayerTurn::OnMontageInterrupted);
	PlayTask->OnCancelled.AddDynamic(this, &UKDGameplayAbility_PlayerTurn::OnMontageCancelled);
	PlayTask->OnBlendOut.AddDynamic(this, &UKDGameplayAbility_PlayerTurn::OnMontageBlendOut);
	PlayTask->ReadyForActivation();

	StartSafetyTimer(SelectedMontage->GetPlayLength(), MontagePlayRate);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UKDGameplayAbility_PlayerTurn::OnCleanup(bool bWasCancelled)
{
	// 회전 모드 복원 — 모든 종료 경로(완료/캔슬/피격) 공통이라 leak 없음.
	if (bRestoreOrientRotation)
	{
		if (ACharacter* TurnChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if (UCharacterMovementComponent* MoveComp = TurnChar->GetCharacterMovement())
			{
				MoveComp->bOrientRotationToMovement = true;
			}
		}
		bRestoreOrientRotation = false;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(GameplayTags::State_Movement_Turning);
	}
}

void UKDGameplayAbility_PlayerTurn::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKDGameplayAbility_PlayerTurn::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKDGameplayAbility_PlayerTurn::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKDGameplayAbility_PlayerTurn::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
