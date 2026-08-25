// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"
#include "GA_SprintAttack.generated.h"

// 달리기 공격 — 단발. 몽타주 = 에디터의 AttackMontage
UCLASS()
class PROJECT_KD_API UGA_SprintAttack : public UGA_PlayerMeleeAttackBase
{
	GENERATED_BODY()

public:
	UGA_SprintAttack();
};
