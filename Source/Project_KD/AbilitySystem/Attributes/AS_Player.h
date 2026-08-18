// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_Player.generated.h"

// 플레이어 전용 어트리뷰트 — Stamina | Ammo

UCLASS()
class PROJECT_KD_API UAS_Player : public UAS_CharacterBase
{
	GENERATED_BODY()

public:
	UAS_Player();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	// 스킬 코스트 자원
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAS_Player, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UAS_Player, MaxStamina)

	// 사격 탄약 — 소모 = GA_Shoot 의 Cost GE
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UAS_Player, Ammo)
	
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UAS_Player, MaxAmmo)
};
