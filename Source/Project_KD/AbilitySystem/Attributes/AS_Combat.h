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

private:
	// 공격자가 정면 반구 안인지 — OutHitAngle = 정면 기준 부호각 (우 = + | 좌 = -)
	bool IsFrontalAttack(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC, float& OutHitAngle) const;

	// 패링 가로채기 — 퍼펙트 | 적 방어형. 반환 = 데미지를 0으로 삼키고 끝냈는지
	bool TryInterceptByParry(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC,
		bool bFrontalAttack, bool bUnblockable, float HitAngle) const;

	// 경감 — Defense 차감 후 Shield 흡수. 반환 = 체력으로 갈 몫
	float ApplyMitigation(UAbilitySystemComponent* ASC, float Damage) const;

	// 히트리액션 신호 — bBlockedHit = 방어 중 피격 유무
	void SendHitReact(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC, bool bBlockedHit) const;
};
