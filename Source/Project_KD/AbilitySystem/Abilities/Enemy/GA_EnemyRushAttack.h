// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/GA_EnemyWeaponTraceBase.h"
#include "GA_EnemyRushAttack.generated.h"

// 도끼 엘리트 gap-closer(돌진). 발동 시 Motion Warp target을 플레이어 쪽으로 설정 →
// 루트모션 몽타주가 그 지점까지 돌진. 단발 조준(리딩 없음) — 플레이어가 크게 움직이면 빗나감(회피 보상).
UCLASS()
class PROJECT_KD_API UGA_EnemyRushAttack : public UGA_EnemyWeaponTraceBase
{
	GENERATED_BODY()

protected:
	// 몽타주의 MotionWarping 노티에 지정한 Warp Target Name과 일치해야 함.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Rush")
	FName WarpTargetName = TEXT("RushTarget");

	// 플레이어 위치에서 이만큼 앞에 목표 설정(겹침 방지, 도끼 리치 확보).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Rush", meta = (ClampMin = "0.0"))
	float RushStopDistance = 200.f;

	// 워프 최대 거리(cm). 초과 시 여기까지만 돌진하고 빗나감(루트모션 과신장 방지).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Rush", meta = (ClampMin = "0.0"))
	float MaxRushWarpDistance = 600.f;

	virtual void OnActivated() override;
	// AI Focus 해제 백스톱(캔슬 경로 포함) + base 정리.
	virtual void OnCleanup(bool bWasCancelled) override;

private:
	// 플레이어 현재 위치 기준으로 돌진 종점을 계산해 warp target에 push.
	void UpdateWarpTarget();

	// Event.Rush.Warp 수신 → 돌진 직전 위치로 재조준(윈드업 중 오버슈트 방지).
	UFUNCTION()
	void OnRushWarpEvent(FGameplayEventData Payload);
};
