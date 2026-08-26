// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "KDAbilitySystemGlobals.generated.h"

// ASC 가 만드는 기존 Context를 FKDGameplayEffectContext 로 교체
UCLASS()
class PROJECT_KD_API UKDAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

protected:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
