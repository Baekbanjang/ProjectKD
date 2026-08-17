// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_SprintAttack.h"

#include "KDGameplayTags.h"

UGA_SprintAttack::UGA_SprintAttack()
{
	AbilityTags.AddTag(GameplayTags::Ability_Player_SprintAttack);
}
