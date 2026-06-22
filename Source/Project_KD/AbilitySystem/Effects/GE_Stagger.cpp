// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/GE_Stagger.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGE_Stagger::UGE_Stagger()
{
	// Infinite: HandleStagger 적용 → RecoverFromStagger(3s 타이머/페이오프)가 제거.
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// Staggered = 적 GA ActivationBlockedTags 차단.
	UTargetTagsGameplayEffectComponent* TagsComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComp"));
	FInheritedTagContainer InheritableTags;
	InheritableTags.Added.AddTag(GameplayTags::State_Combat_Staggered);
	TagsComp->SetAndApplyTargetTagChanges(InheritableTags);
	GEComponents.Add(TagsComp);

	// 스턴 비주얼 — GC_Staggered(BP)가 OnActive/OnRemove로 GE 수명에 자동 동기화.
	FGameplayEffectCue StaggerCue;
	StaggerCue.GameplayCueTags.AddTag(GameplayTags::GameplayCue_Combat_Staggered);
	GameplayCues.Add(StaggerCue);
}
