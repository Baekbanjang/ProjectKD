// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Combat/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/AnimNotifies/ANS_MeleeTrace.h"
#include "Combat/Data/HitConfirmProfile.h"


void UGA_PlayerMeleeAttackBase::OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit)
{
	UAbilitySystemComponent* AttackerASC = GetAbilitySystemComponentFromActorInfo();
	if (!AttackerASC || !TargetASC) return;
	
	// i-frame 닷지/사망한 대상엔 타격감 큐 생략
	if (TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Invulnerable)
		|| TargetASC->HasMatchingGameplayTag(GameplayTags::State_Dead))
	{
		return;
	}
	
	// 큐가 ImpactPoint에 정확히 찍히도록 컨텍스트에 HitResult 동봉
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());
	Context.AddHitResult(Hit);
	
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;
	CueParams.RawMagnitude = HitConfirmMagnitude;
	CueParams.EffectContext = Context;
	CueParams.SourceObject = HitConfirmProfile;
	
	AttackerASC->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_PlayerHitConfirm, CueParams);

	// 히트스탑 윈도우 확보 - bIgnoreHitStop 확인 후 생략 가능
	const UANS_MeleeTrace* Window = GetActiveWindow();
	if (Window && Window->bIgnoreHitStop) { return; }
	
	// 정지 주체는 공격자 HitStopComponent
	FGameplayEventData StopEvent;
	StopEvent.EventMagnitude = AttackerHitStopDuration;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetAvatarActorFromActorInfo(), GameplayTags::Event_Combat_HitStop, StopEvent);
}

void UGA_PlayerMeleeAttackBase::OnActivated()
{
	ACharacter* Attacker = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Attacker) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn)) return;
	
	ULockOnComponent* LockOn = GetLockOnComponentFromActorInfo();
	if (!LockOn) return;

	AActor* Target = LockOn->GetLockedTarget();
	if (!Target) return;
	
	// 락온 자동 조준 — 뒤쪽 135도 초과는 제외
	const FVector ToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;
	
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Attacker->GetActorRotation().Yaw, ToTarget.Rotation().Yaw);
	if (FMath::Abs(DeltaYaw) <= 135.f)
		Attacker->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
}
