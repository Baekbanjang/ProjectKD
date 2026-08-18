// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/AS_Player.h"
#include "KDGameplayTags.h"
#include "GameplayEffectExtension.h"

UAS_Player::UAS_Player()
{
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	
	InitAmmo(30.0f);
	InitMaxAmmo(30.0f);
}

void UAS_Player::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
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
			// 재장전
			if (GetAmmo() <= 0.0f)
			{
				ASC->SetLooseGameplayTagCount(GameplayTags::State_Gun_Reloading, 1);
			}
			// 충전 시 해제
			else if (GetAmmo() >= GetMaxAmmo())
			{
				ASC->SetLooseGameplayTagCount(GameplayTags::State_Gun_Reloading, 0);
			}
		}
	}
}

