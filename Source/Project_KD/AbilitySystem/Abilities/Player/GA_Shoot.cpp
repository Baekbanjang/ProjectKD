// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_Shoot.h"

#include "KDGameplayTags.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Combat/KDProjectile.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

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

	// 발사 프레임 = 몽타주 노티
	UAbilityTask_WaitGameplayEvent* ShootTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_Shoot, nullptr, false, true);
	ShootTask->EventReceived.AddDynamic(this, &UGA_Shoot::OnShootEvent);
	ShootTask->ReadyForActivation();
	
	// 몽타주 콜백 유실 시 EndAbility
	StartSafetyTimer(ShootMontage->GetPlayLength(), MontagePlayRate);
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Shoot::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Shoot::OnShootEvent(FGameplayEventData Payload)
{
	// 취소 체인이 노티보다 먼저 끝냈을 수 있음
	if (!IsActive()) return;
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = GetWorld();
	if (!IsValid(Avatar) || !ASC || !World) return;
	
	if (!ensureMsgf(ProjectileClass && DamageEffectClass,
		TEXT("[KD] %s — Projectile Class | Damage Effect Class 미지정"), *GetName()))
	{
		return;
	}
	
	// 총구 위치
	const FVector MuzzleLoc = UKDAbilityStatics::GetMuzzleTransform(Avatar, MuzzleSocket, WeaponTag).GetLocation();
	
	// 시점 = 카메라 실제 값  컨트롤 피치는 레일 눈금이라 시선 각도 X
	FVector ViewLoc = MuzzleLoc;
	FRotator ViewRot = Avatar->GetActorRotation();
	if (const APlayerController* PC = Cast<APlayerController>(Avatar->GetController()))
	{
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);
	}
	
	// 조준점 = 화면 중앙 트레이스
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Avatar);   // 카메라와 총구 사이에 플레이어 아바타 존재
	
	const FVector TraceEnd = ViewLoc + ViewRot.Vector() * AimTraceRange;
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, ViewLoc, TraceEnd, ECC_Visibility, TraceParams);
	const FVector AimPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	
	// 총구에서 조준점으로
	const FRotator SpawnRot = (AimPoint - MuzzleLoc).Rotation();
	
	// 데미지 Spec
	UKDAbilityStatics::SpawnDamageProjectile(
		ASC, Avatar, ProjectileClass, DamageEffectClass, MuzzleLoc, SpawnRot, GetAssetTags());
}
