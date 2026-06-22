// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerAttackBase.h"
#include "GA_HeavyAttack.generated.h"

// 강공 GA. 생성자에서 ComboInputTag = Input.Combo.Heavy 지정.
// 콤보 처리/타격감/락온 자동조준은 전부 GA_PlayerAttackBase가 담당.
UCLASS()
class PROJECT_KD_API UGA_HeavyAttack : public UGA_PlayerAttackBase
{
	GENERATED_BODY()

public:
	UGA_HeavyAttack();

	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) override;

protected:
	// 강공 1회(=콤보 매 강공 입력) 스태미나 소모량.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Heavy", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float HeavyStaminaCost = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Stamina")
	TSubclassOf<UGameplayEffect> StaminaCostGE;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Stamina")
	TSubclassOf<UGameplayEffect> StaminaRegenBlockGE;

};
