// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"
#include "GA_PlayerOneShotAttack.generated.h"

// 단발 계열 플레이어 공격 GA (달리기 공격, 반격 찌르기 등)
// 몽타주는 에디터의 AttackMontage 칸을 그대로 사용
// 몽타주가 여러 개가 되거나 분기가 생기면 UGA_PlayerAttackBase(콤보 계열)로 옮길 것
// 타격감/락온은 전부 부모가 함 — 이 클래스는 분류 표시 역할
UCLASS()
class PROJECT_KD_API UGA_PlayerOneShotAttack : public UGA_PlayerMeleeAttackBase
{
	GENERATED_BODY()
};
