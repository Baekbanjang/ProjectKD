// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Enemy/GA_EnemyWeaponTraceBase.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Enemy/AI/EncounterSubsystem.h"
#include "Enemy/KDEnemyBaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_EnemyWeaponTraceBase::UGA_EnemyWeaponTraceBase()
{
	// GA-레벨 once-per-actor를 끔: 다단 공격에서 스윙마다 패링 게이트 도달 보장. tick-spam은 per-window 태스크가 막음.
	bOncePerActor = false;
	WeaponMeshComponentTag = TEXT("Weapon");
}

void UGA_EnemyWeaponTraceBase::OnActivated()
{
	// 공격 시작 시 제자리 플랜트: 추격 MoveTo 잔여 속도(braking glide)가 몽타주 중 미끄러지는 걸 막음.
	if (APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (AAIController* AICon = Cast<AAIController>(Pawn->GetController()))
		{
			AICon->StopMovement();
		}
		if (UCharacterMovementComponent* Move = Pawn->FindComponentByClass<UCharacterMovementComponent>())
		{
			Move->StopMovementImmediately();
		}
	}

	// 전조 타이밍은 몽타주 윈드업에 배치한 ANS_TelegraphWindow가 제어. OnCleanup의 RemoveGameplayCue는 취소 안전망.
}

float UGA_EnemyWeaponTraceBase::GetEffectiveMontagePlayRate() const
{
	AKDEnemyBaseCharacter* Char = Cast<AKDEnemyBaseCharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Char))
		return Super::GetEffectiveMontagePlayRate();
	return Super::GetEffectiveMontagePlayRate() * Char->GetAttackSpeedMultiplier();
}

void UGA_EnemyWeaponTraceBase::OnCleanup(bool bWasCancelled)
{
	if (TelegraphCueTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveGameplayCue(TelegraphCueTag);
		}
	}

	// 토큰 반납 — 원거리 kiter처럼 토큰 미보유인 경우 ReturnToken은 no-op.
	if (AKDEnemyBaseCharacter* Char = Cast<AKDEnemyBaseCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UWorld* World = Char->GetWorld())
		{
			if (UEncounterSubsystem* ES = World->GetSubsystem<UEncounterSubsystem>())
				ES->ReturnToken(Char);
		}
	}

	// Chain base trace cleanup (required per GA_WeaponTraceBase contract).
	Super::OnCleanup(bWasCancelled);
}
