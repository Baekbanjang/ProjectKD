// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KDAbilityStatics.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
// GAS 공용 static 헬퍼
UCLASS()
class PROJECT_KD_API UKDAbilityStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TryConsumeStamina(
		UAbilitySystemComponent* ASC,
		TSubclassOf<UGameplayEffect> StaminaCostGE,
		TSubclassOf<UGameplayEffect> StaminaRegenBlockGE,
		float Cost);
};
