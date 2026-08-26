// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_AmmoCost.h"

#include "AbilitySystem/Attributes/KDPlayerAttributeSet.h"

UKDGameplayEffect_AmmoCost::UKDGameplayEffect_AmmoCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	// 소모량 = 1발 고정
	FGameplayModifierInfo AmmoMod;
	AmmoMod.Attribute = UKDPlayerAttributeSet::GetAmmoAttribute();
	AmmoMod.ModifierOp = EGameplayModOp::Additive;
	AmmoMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-1.0f));
	Modifiers.Add(AmmoMod);
}
