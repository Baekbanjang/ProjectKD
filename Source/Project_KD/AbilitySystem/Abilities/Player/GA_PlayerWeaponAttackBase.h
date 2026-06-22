// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_WeaponTraceBase.h"
#include "GA_PlayerWeaponAttackBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECT_KD_API UGA_PlayerWeaponAttackBase : public UGA_WeaponTraceBase
{
	GENERATED_BODY()

protected:
	// 플레이어 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.08f;

	// 적 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float VictimHitStopDuration = 0.12f;

	// 히트 스탑
	void ApplyHitStop(AActor* Target, float Duration) const;
};
