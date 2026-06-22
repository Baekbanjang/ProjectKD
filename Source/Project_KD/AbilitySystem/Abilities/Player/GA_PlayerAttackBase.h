// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerWeaponAttackBase.h"
#include "GA_PlayerAttackBase.generated.h"

class UHitConfirmProfile;
class UAnimMontage;
class UGameplayEffect;

// 플레이어 공격 GA 공통 베이스. 순수 공용 WeaponTraceBase 위에 플레이어 전용을 얹음:
// 콤보 입력 처리(ActivateAbility) + 타격감 큐(OnTargetHit) + 락온 자동조준(OnActivated).
// 자식(Light/Heavy/Charge)은 생성자에서 ComboInputTag만 지정.
UCLASS(Abstract)
class PROJECT_KD_API UGA_PlayerAttackBase : public UGA_PlayerWeaponAttackBase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// 자식 생성자가 지정하는 콤보 입력 태그(Light/Heavy/HeavyCharge). ComboComponent::ProcessInput에 전달.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Combo")
	FGameplayTag ComboInputTag;

	// 1~3타용 디폴트 단발 Montage. GA 인스턴스가 활성화 사이에 살아남아 직전 분기 Montage가 잔류 → 매 시작에 복원.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TArray<TObjectPtr<UAnimMontage>> DefaultAttackMontages;

	// 1~3타용 디폴트 데미지 GE. 동일 이유로 매번 복원.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectClass;

	// 타격감 큐 크기(셰이크/이펙트 스케일). 플레이어 전용이라 WeaponTraceBase에서 내려옴.
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;

	// 무기별 타격감 프로필 — CueParams.SourceObject로 전달, BP GC가 무기별 분기.
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UHitConfirmProfile> HitConfirmProfile;

	// 타격 시 플레이어 전용 HitConfirm 큐 실행.
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) override;

	// 락온 중 공격 시 타겟 방향 자동 조준.
	virtual void OnActivated() override;

	virtual float GetEffectiveMontagePlayRate() const override;
};
