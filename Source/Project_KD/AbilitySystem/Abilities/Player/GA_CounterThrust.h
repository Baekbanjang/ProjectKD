// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerOneShotAttack.h"
#include "GA_CounterThrust.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_KD_API UGA_CounterThrust : public UGA_PlayerOneShotAttack
{
	GENERATED_BODY()

public:
	UGA_CounterThrust();

protected:
	virtual void OnActivated() override;

	// 몽타주의 Motion Warping 노티 동기점 이름과 일치
	UPROPERTY(EditDefaultsOnly, Category = "Action|Warp")
	FName WarpTargetName = TEXT("CounterTarget");

	// 타겟 앞에서 멈출 거리(cm)
	UPROPERTY(EditDefaultsOnly, Category = "Action|Warp", meta = (ClampMin = "0.0"))
	float WarpStopDistance = 150.f;

	// 락온 없을 때 이 반경 안에서만 대시 대상 탐색
	UPROPERTY(EditDefaultsOnly, Category = "Action|Warp", meta = (ClampMin = "100.0"))
	float MaxDashRange = 800.f;
	
};
