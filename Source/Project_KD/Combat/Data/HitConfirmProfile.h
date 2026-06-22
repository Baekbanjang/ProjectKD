// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitConfirmProfile.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECT_KD_API UHitConfirmProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 무기/스킬별 타격음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitConfirm")
	TObjectPtr<USoundBase> HitSound;

	// 충돌 지점에 스폰하는 임팩트 파티클
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitConfirm")
	TObjectPtr<UNiagaraSystem> ImpactVFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitConfirm")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;
};
