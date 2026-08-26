// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerMelee.h"
#include "KDGameplayAbility_CounterThrust.generated.h"

// 퍼펙트 회피 반격 찌르기 — 단발 + 모션워핑 대시
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_CounterThrust : public UKDGameplayAbility_PlayerMelee
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_CounterThrust();

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
