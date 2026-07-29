// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"

#include "AbilitySystemComponent.h"
#include "Combat/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "Player/KDPlayerCharacter.h"
#include "KDGameplayTags.h"
#include "Combat/Data/HitConfirmProfile.h"

void UGA_PlayerMeleeAttackBase::ApplyHitStop(AActor* Target, float Duration) const
{
	ACharacter* Char = Cast<ACharacter>(Target);
	
	if (!Char || Duration <= 0.f) return;
	USkeletalMeshComponent* Mesh = Char->GetMesh();
	UAnimInstance* AnimInst = Mesh ? Mesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurMontage = AnimInst ? AnimInst->GetCurrentActiveMontage() : nullptr;
	
	if (!AnimInst || !CurMontage) return;
	// SetPlayRate(0) 대신 Pause — 겹친 두 번째 타격이 rate=0을 물어 영구 정지하는 것 방지
	AnimInst->Montage_Pause(CurMontage);

	TWeakObjectPtr<UAnimInstance> WeakAnim(AnimInst);
	TWeakObjectPtr<UAnimMontage> WeakMontage(CurMontage);
	
	FTimerHandle TH;
	Char->GetWorldTimerManager().SetTimer(TH,
		FTimerDelegate::CreateLambda([WeakAnim, WeakMontage]()
		{
			if (WeakAnim.IsValid() && WeakMontage.IsValid())
			{
				WeakAnim->Montage_Resume(WeakMontage.Get());
			}
		}), Duration, false);
}

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
	// 플레이어 공격에만 히트스탑 — 공격자(플레이어) + 피격자(적)
	ApplyHitStop(GetAvatarActorFromActorInfo(), AttackerHitStopDuration);
	ApplyHitStop(HitActor, VictimHitStopDuration);
}

void UGA_PlayerMeleeAttackBase::OnActivated()
{
	ACharacter* Attacker = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Attacker) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn)) return;
	
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(Attacker);
	if (!PC || !PC->GetLockOnComponent()) return;
	
	AActor* Target = PC->GetLockOnComponent()->GetLockedTarget();
	if (!Target) return;
	
	// 락온 자동 조준 — 뒤쪽 135도 초과는 제외
	const FVector ToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;
	
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Attacker->GetActorRotation().Yaw, ToTarget.Rotation().Yaw);
	if (FMath::Abs(DeltaYaw) <= 135.f)
		Attacker->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
}
