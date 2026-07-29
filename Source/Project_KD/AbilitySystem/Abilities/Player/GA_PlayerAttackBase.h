// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"
#include "GA_PlayerAttackBase.generated.h"

class UGameplayEffect;

// 콤보 계열 플레이어 공격 GA
// 몽타주는 DA_ComboTree 노드가 줌 — ActivateAbility가 AttackMontage를 덮어씀
// 그래서 Action|Montage 카테고리를 디테일에서 숨김
// 타격감/락온은 부모(UGA_PlayerMeleeAttackBase)가 함
// 자식(Light/Heavy)은 생성자에서 ComboInputTag만 지정

UCLASS(Abstract, HideCategories = ("Action|Montage"))
class PROJECT_KD_API UGA_PlayerAttackBase : public UGA_PlayerMeleeAttackBase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// 자식 생성자가 지정하는 콤보 입력 태그 — ComboComponent::ProcessInput에 넘김
	UPROPERTY(EditDefaultsOnly, Category = "Action|Combo")
	FGameplayTag ComboInputTag;

	
	// 노드가 GE를 안 주면 이 값 — InstancedPerActor라 직전 값이 남음, 매 시작 복원
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectClass;
};
