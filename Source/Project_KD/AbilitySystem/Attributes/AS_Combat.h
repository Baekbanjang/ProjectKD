// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_Combat.generated.h"

// 전투 어트리뷰트 — 데미지 단일 관문
UCLASS()
class PROJECT_KD_API UAS_Combat : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAS_Combat();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 데미지 분배 — IncomingDamage 소진 | 패링 | Defense 경감 | Shield 경감 | Health 차감
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAS_Combat, AttackPower)

	// 데미지 차감값 — 음수 X
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UAS_Combat, Defense)

	// 메타 어트리뷰트 — 들어온 데미지를 담는 1회용 버킷. 영속 X | 복제 X
	UPROPERTY()
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAS_Combat, IncomingDamage)
};
