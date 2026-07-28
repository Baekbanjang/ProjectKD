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

	const FComboNode* Node = IsValid(Combo)
		? Combo->ProcessInput(ComboInputTag)
		: nullptr;

	// 매 시작에 디폴트 복원 — 직전 분기 값이 다음 활성화에 잔류하는 것 차단.
	DamageEffectClass = DefaultDamageEffectClass;

	if (Node)
	{
		// 노드 = 이번 콤보
		AttackMontage = IsValid(Node->Montage) ? Node->Montage : nullptr; // 노드의 몽타주 GA 변수에 대입
		if (!AttackMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[KD] Combo node '%s' 몽타주 미지정"), *Node->NodeId.ToString());
		}
		
		if (Node->DamageEffectClass)
		{
			DamageEffectClass = Node->DamageEffectClass;
		}
	}
	else
	{
		// 다음 노드 없음 -> 기본 배열로 대신
		const int32 Num = DefaultAttackMontages.Num();
		if (Num > 0)
		{
			const int32 Length = FMath::Max(IsValid(Combo) ? Combo->GetComboDepth() : 1, 1);
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

/*
float UGA_PlayerAttackBase::GetEffectiveMontagePlayRate() const
{
	AKDPlayerCharacter* Player = Cast<AKDPlayerCharacter>(GetAvatarActorFromActorInfo());
	const UComboComponent* Combo = IsValid(Player) ? Player->GetComboComponent() : nullptr;
	const float Mult = IsValid(Combo) ? Combo->GetTempoMultiplier() : 1.f;
	const float Eff = MontagePlayRate * Mult;
	return Eff;
}*/
