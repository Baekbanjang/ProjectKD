// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "AbilitySystem/Attributes/KDAttributeAccessors.h"
#include "AS_Player.generated.h"

// 길동(플레이어) 전용 어트리뷰트 — Stamina(회피·강공)와 Dosul(도술) 보유.
// 적/보스는 본 클래스 미사용 (UAS_CharacterBase + UAS_Combat만).
UCLASS()
class PROJECT_KD_API UAS_Player : public UAS_CharacterBase
{
	GENERATED_BODY()

public:
	UAS_Player();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	// 회피(Shift) / 강공(H 차징) 소모 자원. 적은 고정 모션/쿨다운으로 처리되어 미보유.
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAS_Player, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UAS_Player, MaxStamina)

	// 도술(화부술/풍보/정승) 발동 자원. 적중·패링으로 충전, GA 발동 시 소모.
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Dosul;
	ATTRIBUTE_ACCESSORS(UAS_Player, Dosul)

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxDosul;
	ATTRIBUTE_ACCESSORS(UAS_Player, MaxDosul)
};
