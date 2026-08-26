// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "KDGameplayEffect_StaminaCost.generated.h"

/**
 * 
 */

// Instant GE: Stamina += SetByCaller(SetByCaller.Stamina). 호출자가 음수 magnitude 전달(GE_Damage_Physical 동일) 
// GA의 CostGameplayEffectClass로 등록 시 ASC가 "Stamina < 소모량" 자동 reject(CanActivateAbility 단계).
UCLASS()
class PROJECT_KD_API UKDGameplayEffect_StaminaCost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UKDGameplayEffect_StaminaCost();
};
