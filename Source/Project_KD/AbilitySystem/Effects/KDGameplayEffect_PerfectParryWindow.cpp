// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_PerfectParryWindow.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UKDGameplayEffect_PerfectParryWindow::UKDGameplayEffect_PerfectParryWindow()
{
	// 퍼펙트 윈도우 = State.Combat.PerfectParryReady 부여
	// 길이는 이 GE 의 Duration — 조절은 GE_PerfectParry 에셋에서
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.2f));

	UTargetTagsGameplayEffectComponent* TagsComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComp"));
	FInheritedTagContainer InheritableTags;
	InheritableTags.Added.AddTag(GameplayTags::State_Combat_PerfectParryReady);
	TagsComp->SetAndApplyTargetTagChanges(InheritableTags);
	GEComponents.Add(TagsComp);
}
