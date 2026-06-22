// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ExecutionProfile.generated.h"

class UAnimMontage;
class UGameplayEffect;

// 적 한 종류의 처형 정의. BP 에셋으로 생성 후 ExecutionComponent에 할당.
// 잡몹 즉사(bSurvivable=false) / 엘리트 생존(bSurvivable=true, 칩 데미지) 분기.
UCLASS(BlueprintType)
class PROJECT_KD_API UExecutionProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 생존 처형 모션(다운→기상). 데스블로(치명)가 아닐 때 재생. ExecutionComponent.OnExecutionBegin을 받은 Pawn이 재생.
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UAnimMontage> Montage;

	// 처치(데스블로) 모션(다운→사망, 기상 없음). 이 처형이 적을 죽일 때 재생 → 끝나면 랙돌. 미지정이면 Montage 폴백.
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UAnimMontage> DeathblowMontage;

	// 생존 처형 칩 데미지. 미지정이면 HP는 일반 공격으로만 감소.
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TSubclassOf<UGameplayEffect> SurviveDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	FGameplayTag CueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Execution", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float Duration = 1.5f;

	// false(잡몹)=항상 데스블로. true(엘리트)=Health<=Threshold일 때만 데스블로, 아니면 생존(다운→기상).
	// 처형 시작 시점에 판정 — 모션 선택보다 먼저.
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	bool bSurvivable = false;

	// 데스블로 임계 Health(bSurvivable=true에서만 사용). 처형 시작 시 Health가 이 값 이하면 치명 처형.
	UPROPERTY(EditDefaultsOnly, Category = "Execution", meta = (ClampMin = "0.0"))
	float DeathblowHealthThreshold = 0.f;
};
