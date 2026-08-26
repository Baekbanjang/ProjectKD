// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "KDPlayerExecutionProfile.generated.h"

class ULevelSequence;
class UAnimMontage;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDPlayerExecutionProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 플레이어 피니셔 몽타주(공격자)
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UAnimMontage> FinisherMontage;

	// 시네 카메라 큐
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	FGameplayTag CameraCueTag;

	// 처형 시네마틱(레벨 시퀀스)
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<ULevelSequence> FinisherSequence;   // ★ 추가

	// 적 처형 발동 가능 거리
	UPROPERTY(EditDefaultsOnly, Category = "Execution", meta = (ClampMin = "0"))
	float GrappleDistance = 150.f;
};
