// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_SprintAttack.h"

#include "KDGameplayTags.h"

UKDGameplayAbility_SprintAttack::UKDGameplayAbility_SprintAttack()
{
	AbilityTags.AddTag(GameplayTags::Ability_Player_SprintAttack);
}
