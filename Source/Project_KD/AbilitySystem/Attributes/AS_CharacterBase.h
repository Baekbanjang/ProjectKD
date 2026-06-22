// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_CharacterBase.generated.h"

UCLASS()
class PROJECT_KD_API UAS_CharacterBase : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAS_CharacterBase();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, MaxHealth)

	// 균형 게이지 — 적/보스 Stagger 시스템 공통. 0 도달 시 자세 붕괴.
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, Poise)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, MaxPoise)
};
