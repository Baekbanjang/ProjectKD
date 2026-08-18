// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/AS_Player.h"
#include "KDGameplayTags.h"
#include "GameplayEffectExtension.h"

UAS_Player::UAS_Player()
{
	// 기능 : 플레이어 어트리뷰트 초기값 — Shield 는 부모의 0 을 덮음
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	
	InitAmmo(30.0f);
	InitMaxAmmo(30.0f);

	InitShield(50.0f);
	InitMaxShield(50.0f);
}

void UAS_Player::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 기능 : Stamina | Ammo 0 ~ Max 클램프
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
}

void UAS_Player::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	// 기능 : Stamina | Ammo 사후 클램프 + 탄약 0 에서 재장전 태그
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.0f, GetMaxAmmo()));
		
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			// 탄약 0 = 재장전 중
			if (GetAmmo() <= 0.0f)
			{
				ASC->SetLooseGameplayTagCount(GameplayTags::State_Gun_Reloading, 1);
			}
			// 만충 = 해제
			else if (GetAmmo() >= GetMaxAmmo())
			{
				ASC->SetLooseGameplayTagCount(GameplayTags::State_Gun_Reloading, 0);
			}
		}
	}
}

