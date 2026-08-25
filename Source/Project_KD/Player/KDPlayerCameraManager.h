// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "KDPlayerCameraManager.generated.h"

/**
 * - 생성자: SB Eve_CameraPitch/Fov 정합값 (Pitch -80/+45 비대칭, FOV 75)
 * - 커브 슬롯: FovByDistance(거리별 FOV), SlopeControl(경사 보정) — 동적 카메라용
 * - UpdateCamera: 매 프레임 커브 평가해서 POV 후처리
 */
UCLASS()
class PROJECT_KD_API AKDPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AKDPlayerCameraManager();

protected:
	// 카메라 거리(cm) -> FOV(deg). 가까이=좁게(시네마틱), 멀리=넓게(가독성)
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Curves")
	TObjectPtr<UCurveFloat> FovByDistanceCurve;

	// FOV 임팩트 커브: 시간 -> FOV 델타(음수=줌인)
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Impact")
	TObjectPtr<UCurveFloat> FovPunchCurve;
	
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Camera|Impact")
	void TriggerFovPunch(float Scale = 1.f);

private:
	float FovPunchElapsed = -1.f;
	float FovPunchScale = 1.f;
};
