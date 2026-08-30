// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "KDSlowMotionSubsystem.generated.h"

USTRUCT()
struct FKDSlowMoRequest
{
	GENERATED_BODY()
	// 시간 배율 (예 - 시간 0.7배)
	float Scale = 1.f;
	
	// 우선순위 
	int32 Priority = 0;
	
	// 만료 시각 
	float ExpireTime = 0.f;
};
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDSlowMotionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SlowMotion", meta =(ToolTip = "0 = 기본 | 10 = 짧고 강한 히트스톱 | 1000 = 처형·사망"))
	void RequestSlowMo(float Scale, float Duration, int32 Priority = 0);

protected:
	// 레벨 전환시 시간 복구
	virtual void Deinitialize() override;

private:
	// 완료한 요청 제거 후, Priority 적용
	void Recalculate();
	
	// 활성 요청 
	TArray<FKDSlowMoRequest> Requests;
	
	FTimerHandle RecalcTimer;
};
