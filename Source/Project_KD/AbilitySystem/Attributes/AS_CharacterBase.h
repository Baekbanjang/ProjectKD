// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_CharacterBase.generated.h"

// 캐릭터 공용 어트리뷰트 — 플레이어 | 적 공유
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

	// 균형 게이지 — 0 = 자세 붕괴. MaxPoise = 상태 바 칸 수
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, Poise)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, MaxPoise)

	// 피해 경감 자원 — 0 = 경감 X
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UAS_CharacterBase, MaxShield)
};
