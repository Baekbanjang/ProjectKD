// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerMelee.h"
#include "KDGameplayAbility_Skill.generated.h"

/**
 * 캐릭터 스킬 공용 베이스 - 단발
 */
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_Skill : public UKDGameplayAbility_PlayerMelee
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_Skill();

protected:
	// 발동 시 소모 스태미나
	UPROPERTY(EditDefaultsOnly, Category = "Action|Cost", meta = (ClampMin = "0.0"))
	float StaminaCost = 10.f;

	// 스태미나 검사
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	// 스태미나 소모
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;
};
