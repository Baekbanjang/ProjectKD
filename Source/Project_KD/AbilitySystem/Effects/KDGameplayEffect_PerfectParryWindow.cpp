// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_PerfectParryWindow.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UKDGameplayEffect_PerfectParryWindow::UKDGameplayEffect_PerfectParryWindow()
{
	// 기본 0.2s 퍼펙트 윈도우: Target에 State.Combat.PerfectParryReady 부여.
	// GA_Parry가 Spec.Data->SetDuration(PerfectParryWindowSec, true)로 길이 덮음.
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.2f));

	UTargetTagsGameplayEffectComponent* TagsComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComp"));
	FInheritedTagContainer InheritableTags;
	InheritableTags.Added.AddTag(GameplayTags::State_Combat_PerfectParryReady);
	TagsComp->SetAndApplyTargetTagChanges(InheritableTags);
	GEComponents.Add(TagsComp);
}
