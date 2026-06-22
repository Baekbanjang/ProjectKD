// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ComboTreeDataAsset.generated.h"

/**
 * 
 */

class UAnimMontage;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EComboContext : uint8
{
	Ground = 0  UMETA(DisplayName = "Ground"),
	Air    = 1  UMETA(DisplayName = "Air"),
};

USTRUCT(BlueprintType)
struct FComboBranch
{
	GENERATED_BODY()

	// 디버그/식별용 ("Incursion_I", "Onslaught_II" 등).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName BranchName;

	// 입력 시퀀스 (예: [Input.Combo.Light, Input.Combo.Heavy, Input.Combo.Heavy, Input.Combo.Heavy] = Incursion I).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FGameplayTag> InputSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};

UCLASS(BlueprintType)
class PROJECT_KD_API UComboTreeDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FComboBranch> Branches;
};
