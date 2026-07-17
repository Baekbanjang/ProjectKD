// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_MeleeTraceBase.h"
#include "GA_PlayerMeleeAttackBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PROJECT_KD_API UGA_PlayerMeleeAttackBase : public UGA_MeleeTraceBase
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
