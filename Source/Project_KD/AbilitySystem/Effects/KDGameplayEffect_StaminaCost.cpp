// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_StaminaCost.h"

#include "KDGameplayTags.h"
#include "AbilitySystem/Attributes/KDPlayerAttributeSet.h"

UKDGameplayEffect_StaminaCost::UKDGameplayEffect_StaminaCost()
{
	// Instant: Stamina += SetByCaller(SetByCaller.Stamina).
	// 호출자가 음수 전달, UAS_Player::PreAttributeChange가 0~Max 자동 클램프.
	
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo StaminaMod;
	StaminaMod.Attribute = UKDPlayerAttributeSet::GetStaminaAttribute();
	StaminaMod.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = GameplayTags::SetByCaller_Stamina;
	StaminaMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(StaminaMod);
}
