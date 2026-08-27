// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "KDTargetFilter.generated.h"

// 타겟 수집 도형
UENUM(BlueprintType)
enum class EKDTargetShapeType : uint8
{
	Arc			UMETA(DisplayName = "부채꼴"),
	Cylinder	UMETA(DisplayName = "원기둥")
};

// 타겟 선택 기준
UENUM(BlueprintType)
enum class EKDTargetSortType : uint8
{
	Nearest			UMETA(DisplayName = "최단거리"),
	SmallestAngle	UMETA(DisplayName = "각도순")
};

// 각도 기준 벡터
UENUM(BlueprintType)
enum class EKDTargetBasisType : uint8
{
	Camera				UMETA(DisplayName = "플레이어 카메라"),
	CharacterForward	UMETA(DisplayName = "캐릭터 정면")
};

// 대상 탐색 조건
USTRUCT(BlueprintType)
struct FKDTargetFilter
{
	GENERATED_BODY()

	// 수집 도형
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter")
	EKDTargetShapeType ShapeType = EKDTargetShapeType::Arc;

	// 탐색 반지름
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
	float Radius = 500.f;

	// 부채꼴 반각
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter",
		meta = (ClampMin = "5.0", ClampMax = "135.0",
			EditCondition = "ShapeType == EKDTargetShapeType::Arc", EditConditionHides))
	float HalfAngle = 90.f;

	// 세로 높이
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter", meta = (ClampMin = "50.0", ClampMax = "3000.0"))
	float Height = 500.0f;

	// 세로 시작 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter", meta = (ClampMin = "-1000.0", ClampMax = "1000.0"))
	float HeightOffset = -150.f;

	// 반각 기준 벡터
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter")
	EKDTargetBasisType Basis = EKDTargetBasisType::CharacterForward;
	
	// 후보 중 선택 기준
	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter")
	EKDTargetSortType SortType = EKDTargetSortType::Nearest;

	UPROPERTY(EditDefaultsOnly, Category = "TargetFilter")
	bool bDrawDebug = false;
};
