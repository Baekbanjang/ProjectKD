// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerAttackBase.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "AbilitySystem/Combo/ComboTreeDataAsset.h"
#include "Combat/LockOnComponent.h"
#include "Combat/Data/HitConfirmProfile.h"
#include "Player/KDPlayerCharacter.h"
#include "GameFramework/Character.h"

void UGA_PlayerAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 적이 이 GA 활용 시 Cast 실패 -> Combo == nullptr -> 디폴트 동작.
	AKDPlayerCharacter* Player = Cast<AKDPlayerCharacter>(GetAvatarActorFromActorInfo());
	UComboComponent* Combo = IsValid(Player) ? Player->GetComboComponent() : nullptr;

	const FComboBranch* Matched = IsValid(Combo)
		? Combo->ProcessInput(ComboInputTag)
		: nullptr;

	// 매 시작에 디폴트 복원 — 직전 분기 값이 다음 활성화에 잔류하는 것 차단.
	DamageEffectClass = DefaultDamageEffectClass;

	if (Matched)
	{
		// 정확 매칭 = 분기 마무리. 분기의 Montage/GE로 교체.
		AttackMontage = IsValid(Matched->Montage) ? Matched->Montage : nullptr;
		if (Matched->DamageEffectClass)
		{
			DamageEffectClass = Matched->DamageEffectClass;
		}
	}
	else
	{
		// 미매칭 = 콤보 진행 중. 현재 길이로 디폴트 배열 인덱싱 (1타째->[0], 2타째->[1], …).
		const int32 Num = DefaultAttackMontages.Num();
		if (Num > 0)
		{
			const int32 Length = FMath::Max(IsValid(Combo) ? Combo->GetInputHistoryLength() : 1, 1);
			const int32 Idx = (Length - 1) % Num;
			AttackMontage = DefaultAttackMontages[Idx];
		}
		else
		{
			// 배열 비어있으면 부모가 EndAbility
			AttackMontage = nullptr;
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_PlayerAttackBase::OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit)
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
	
	// 플레이어 공격에만 히트스탑 — 공격자(플레이어)+피격자(적)
	ApplyHitStop(GetAvatarActorFromActorInfo(), AttackerHitStopDuration);
	ApplyHitStop(HitActor, VictimHitStopDuration);

}

void UGA_PlayerAttackBase::OnActivated()
{
	ACharacter* Attacker = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Attacker) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn)) return;

	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(Attacker);
	if (!PC || !PC->GetLockOnComponent()) return;

	AActor* Target = PC->GetLockOnComponent()->GetLockedTarget();
	if (!Target) return;

	const FVector ToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Attacker->GetActorRotation().Yaw, ToTarget.Rotation().Yaw);
	if (FMath::Abs(DeltaYaw) <= 135.f)
		Attacker->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f)); 
}

float UGA_PlayerAttackBase::GetEffectiveMontagePlayRate() const
{
	AKDPlayerCharacter* Player = Cast<AKDPlayerCharacter>(GetAvatarActorFromActorInfo());
	const UComboComponent* Combo = IsValid(Player) ? Player->GetComboComponent() : nullptr;
	const float Mult = IsValid(Combo) ? Combo->GetTempoMultiplier() : 1.f;
	const float Eff = MontagePlayRate * Mult;
	return Eff;
}
