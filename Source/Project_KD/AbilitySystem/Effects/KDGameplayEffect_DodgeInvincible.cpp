// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_DodgeInvincible.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UKDGameplayEffect_DodgeInvincible::UKDGameplayEffect_DodgeInvincible()
{
	// Duration 0.5s: Target에 State.Combat.Invulnerable 부여. BP 자식에서 DurationMagnitude 오버라이드로 튜닝 가능.
	// CreateDefaultSubobject + GEComponents.Add
	
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.5f));

	UTargetTagsGameplayEffectComponent* TagsComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComp"));
	FInheritedTagContainer InheritableTags;
	InheritableTags.Added.AddTag(GameplayTags::State_Combat_Invulnerable);
	TagsComp->SetAndApplyTargetTagChanges(InheritableTags);
	GEComponents.Add(TagsComp);
}
