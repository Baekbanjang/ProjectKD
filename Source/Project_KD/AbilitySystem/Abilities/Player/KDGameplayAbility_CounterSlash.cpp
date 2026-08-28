// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_CounterSlash.h"
#include "KDGameplayTags.h"

UKDGameplayAbility_CounterSlash::UKDGameplayAbility_CounterSlash()
{
	// 기능 : 태그 교체 
	AbilityTags.Reset();
	AbilityTags.AddTag(GameplayTags::Ability_Player_CounterSlash);
}
