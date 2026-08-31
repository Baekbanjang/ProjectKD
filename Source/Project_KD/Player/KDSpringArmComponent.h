// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "KDSpringArmComponent.generated.h"

class UAbilitySystemComponent;
class USplineComponent;
/**
 * 
 */
UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))

class PROJECT_KD_API UKDSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	UKDSpringArmComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolly")
	TObjectPtr<USplineComponent> DollySpline;

	// 조준 스플라인 — 미지정 시 평상시 스플라인
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dolly")
	TObjectPtr<USplineComponent> AimDollySpline;

	// 조준 모드 블렌드 속도
	UPROPERTY(EditAnywhere, Category = "Dolly", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float AimBlendSpeed = 8.f;
	
	// 스플라인 양 끝 = 시선 피치
	UPROPERTY(EditAnywhere, Category = "Dolly")
	float PitchAtStart = -89.f;   // 스플라인 시작점
	
	UPROPERTY(EditAnywhere, Category = "Dolly")
	float PitchAtEnd = 45.f;      // 스플라인 끝점
	
	UPROPERTY(EditAnywhere, Category = "Dolly")
	float LookAtHeightOffset = 34.f;   // LookAt 지점 = 캡슐 중심 + LookAtHeightOffset = 카메라의 위아래 기울기
	
	// 조준 해제 시 컨트롤 피치를 레일 중앙으로 복귀
	UPROPERTY(EditAnywhere, Category = "Dolly")
	bool bRecenterPitchOnAimExit = true;

	// 복귀 지점 - 0 = 레일 시작 | 0.5 = 가운데 점 | 1 = 레일 끝
	UPROPERTY(EditAnywhere, Category = "Dolly", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimExitRecenterAlpha = 0.5f;
	
	// 카메라 상승 시작 골반 높이
	UPROPERTY(EditAnywhere, Category = "Dolly|Elevate", meta = (ClampMin = "0.0"))
	float PelvisFollowThreshold = 180.f;

	// 시작 높이 초과분을 따라가는 비율
	UPROPERTY(EditAnywhere, Category = "Dolly|Elevate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PelvisFollowRatio = 1.f;

	// 카메라 상승 블렌드 속도
	UPROPERTY(EditAnywhere, Category = "Dolly|Elevate", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float ElevateBlendSpeed = 8.f;

	// 골반 소켓
	UPROPERTY(EditAnywhere, Category = "Dolly|Elevate")
	FName PelvisSocketName = TEXT("pelvis");

	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	
	// 마우스 상하 각도 0~1 변환값  아래 0 | 정면 0.5 | 위 1
	float GetRailAlpha() const { return RailAlpha; }

	// 랙 완화
	UFUNCTION(BlueprintCallable, Category = "Dolly|LagBurst",
		meta = (ToolTip = "TargetLagSpeed 낮을수록 뒤처짐 | Duration 초 | BlendSpeed 값 전환 속도"))
	void RequestLagBurst(float TargetLagSpeed, float TargetMaxDistance, float Duration, float BlendSpeed = 4.f);
private:
	void UpdateAimAlpha(float DeltaTime);   // 조준 태그 -> AimAlpha
	FVector SampleRail(const USplineComponent* Rail, float Alpha) const;   // 스플라인 위 점

	void ApplyRailPosition();     // 스플라인 위 점 -> TargetArmLength | SocketOffset
	void UpdateLookRotation();    // 카메라 위치 기준 LookAt 회전
	
	FQuat RelativeLookRotation = FQuat::Identity;   // GetSocketTransform 캐시
	
	float AimAlpha = 0.f;   // 0 = 평상시 스플라인 | 1 = 조준 스플라인
	float RailAlpha = 0.f;   // ApplyRailPosition 매 틱 갱신
	void RecenterPitchOnAimExit(float DeltaTime);	// 조준 해제시 피치 각도 레일 중앙
	bool bAimActive = false;	// 조준 여부
	
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	void UpdateElevateOffset(float DeltaTime);   // 골반 높이 -> ElevateOffset
	float ElevateOffset = 0.f;   // 카메라 상승량 — 카메라 위치 | LookAt 둘 다 가산

		
	void UpdateLagBurst(float DeltaTime);   // 랙 값 보간 + 만료 복귀
	
	float DefaultLagSpeed = 0.f;       
	float DefaultLagMaxDistance = 0.f;
	float LagBurstSpeed = 0.f;          
	float LagBurstMaxDistance = 0.f;	
	float LagBurstBlendSpeed = 4.f;		// 블렌드 속도
	float LagBurstRemaining = 0.f;		// 복귀 구간
	bool bLagBurstActive = false;       

};
