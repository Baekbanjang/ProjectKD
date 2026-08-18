// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_AmmoCost.h"

#include "AbilitySystem/Attributes/AS_Player.h"

UGE_AmmoCost::UGE_AmmoCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	// 소모량 = 1발 고정
	FGameplayModifierInfo AmmoMod;
	AmmoMod.Attribute = UAS_Player::GetAmmoAttribute();
	AmmoMod.ModifierOp = EGameplayModOp::Additive;
	AmmoMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-1.0f));
	Modifiers.Add(AmmoMod);
}
