// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LockOnConfig.generated.h"

class UCurveFloat;
/**
 * 락온 시스템 파라미터 묶음. ULockOnComponent가 참조해서 검색/카메라/UI 설정 결정.
 * 작성자가 BP에서 DA_LockOnConfig_Default 인스턴스 생성 후 슬롯 할당.
 * DataTable로 마이그레이션 가능 (필터 여러 프리셋).
 */
UCLASS()
class PROJECT_KD_API ULockOnConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 락온 후보 적 검색 최대 거리 (cm). 자동 해제 거리
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Target", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float LockOnRadius = 1000.f;

	// 카메라 forward 기준 시야 콘 각도(degree). 90 = ±45도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Target", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float ViewConeAngle = 90.f;

	// 적까지 거리(cm)별 카메라 상하 각도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Target")
	TObjectPtr<UCurveFloat> LockOnPitchCurve;

	// 카메라 회전 보간 속도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Camera", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float CameraInterpSpeed = 5.f;
	
	// 상하가 커브 값으로 끌려가는 속도
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Camera", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PitchInterpSpeed = 1.5f;

	// 시야 검사(Line of Sight) 활성화 — 장애물 뒤 적은 락온 후보 X, 락온 중 시야 잃으면 즉시 해제.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Target")
	bool bUseLineOfSightCheck = true;

	// 락온 마커 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|UI")
	TSubclassOf<UUserWidget> ReticleWidgetClass;
};
