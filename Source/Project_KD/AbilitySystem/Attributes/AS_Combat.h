// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_Combat.generated.h"

UCLASS()
class PROJECT_KD_API UAS_Combat : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAS_Combat();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 모든 데미지 소스의 단일 관문. IncomingDamage 소진 → Defense 경감 → Health 차감.
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAS_Combat, AttackPower)

	// 데미지 차감용 방어력. 음수 금지 (0 이상). 상한은 GE Damage 계산에서 처리.
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UAS_Combat, Defense)

	// 메타 어트리뷰트 — 데미지 단일 관문(양수 = 들어온 데미지). Transient·비복제·미초기화.
	// GE가 여기에 양수 누적 → PostGameplayEffectExecute가 소진(0)·Defense 경감 후 Health에 적용.
	UPROPERTY()
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAS_Combat, IncomingDamage)
};
