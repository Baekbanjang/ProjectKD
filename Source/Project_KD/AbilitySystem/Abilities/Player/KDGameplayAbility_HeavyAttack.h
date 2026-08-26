// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerCombo.h"
#include "KDGameplayAbility_HeavyAttack.generated.h"

// 강공 GA. 생성자에서 ComboInputTag = Input.Combo.Heavy 지정.
// 콤보 처리/타격감/락온 자동조준은 전부 GA_PlayerAttackBase가 담당.
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_HeavyAttack : public UKDGameplayAbility_PlayerCombo
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_HeavyAttack();
};
