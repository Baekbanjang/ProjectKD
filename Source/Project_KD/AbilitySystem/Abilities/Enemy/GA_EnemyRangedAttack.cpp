// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Enemy/GA_EnemyRangedAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Combat/KDProjectile.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"

void UGA_EnemyRangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsValid(AttackMontage) || !ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// InstigatorTags가 비면 victim 반응이 조용히 no-op — BP 미설정을 발동 시 1회 표면화.
	ensureMsgf(!GetAssetTags().IsEmpty(),
		TEXT("[KD] %s has empty AssetTags — set Ability.Enemy.Grunt.Attack.Ranged on the GA."), *GetName());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage, MontagePlayRate, NAME_None, true, 1.0f);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyRangedAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyRangedAttack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyRangedAttack::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	// 던지는 프레임 = 기존 TraceBegin 노티파이 재사용(신규 이벤트 태그 0).
	UAbilityTask_WaitGameplayEvent* ReleaseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceBegin, nullptr, false, true);
	ReleaseTask->EventReceived.AddDynamic(this, &UGA_EnemyRangedAttack::OnReleaseProjectile);
	ReleaseTask->ReadyForActivation();

	StartSafetyTimer(AttackMontage->GetPlayLength(), MontagePlayRate);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_EnemyRangedAttack::OnReleaseProjectile(FGameplayEventData Payload)
{
	// Race 가드: cancel 체인이 노티파이보다 먼저 EndAbility 했을 수 있음.
	if (!IsActive()) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(Avatar) || !ASC) return;

	if (!ensureMsgf(DamageEffectClass != nullptr, TEXT("[KD] DamageEffectClass not set on %s — set it on the GA."), *GetName()))
	{
		return;
	}

	// 발사 위치 = 메시 소켓(없으면 actor 위치).
	const FVector SpawnLoc = UKDAbilityStatics::GetMuzzleLocation(Avatar, MuzzleSocket);
	FRotator SpawnRot = Avatar->GetActorRotation();

	// 조준 = muzzle→플레이어 직선(높이차/공중 더블점프 대응). actor forward(수평)면 점프한 플레이어를 못 맞춤.
	// 싱글플레이어 1명 가정, 현재위치 조준(리딩 없음). 못 찾으면 actor rotation 폴백(수평).
	if (const APawn* TargetPawn = UGameplayStatics::GetPlayerPawn(Avatar, 0))
	{
		const FVector AimDir = TargetPawn->GetActorLocation() - SpawnLoc;
		if (!AimDir.IsNearlyZero()) SpawnRot = AimDir.Rotation();
	}

	UKDAbilityStatics::SpawnDamageProjectile(
		ASC, Avatar, ProjectileClass, DamageEffectClass, SpawnLoc, SpawnRot, GetAssetTags());
}

void UGA_EnemyRangedAttack::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UGA_EnemyRangedAttack::OnMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}
