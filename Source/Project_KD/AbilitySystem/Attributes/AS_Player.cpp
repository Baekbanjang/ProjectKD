// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/AS_Player.h"

#include "GameplayEffectExtension.h"

UAS_Player::UAS_Player()
{
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	
	InitDosul(0.0f);
	InitMaxDosul(100.0f);
}

void UAS_Player::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	if (Attribute == GetDosulAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxDosul());
	}
}

void UAS_Player::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetDosulAttribute())
	{
		SetDosul(FMath::Clamp(GetDosul(), 0.0f, GetMaxDosul()));
	}
}

