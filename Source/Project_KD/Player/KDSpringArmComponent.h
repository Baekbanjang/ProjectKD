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
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	
private:
	void UpdateAimAlpha(float DeltaTime);   // 조준 태그 -> AimAlpha
	FVector SampleRail(const USplineComponent* Rail, float Alpha) const;   // 스플라인 위 점

	void ApplyRailPosition();     // 스플라인 위 점 -> TargetArmLength | SocketOffset
	void UpdateLookRotation();    // 카메라 위치 기준 LookAt 회전
	
	FQuat RelativeLookRotation = FQuat::Identity;   // GetSocketTransform 캐시
	
	float AimAlpha = 0.f;   // 0 = 평상시 스플라인 | 1 = 조준 스플라인
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
