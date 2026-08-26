// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerMelee.h"
#include "KDGameplayAbility_SprintAttack.generated.h"

// 달리기 공격 — 단발. 몽타주 = 에디터의 AttackMontage
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_SprintAttack : public UKDGameplayAbility_PlayerMelee
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_SprintAttack();
};
