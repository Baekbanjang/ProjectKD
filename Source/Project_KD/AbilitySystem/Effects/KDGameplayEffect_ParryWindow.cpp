// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/KDGameplayEffect_ParryWindow.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UKDGameplayEffect_ParryWindow::UKDGameplayEffect_ParryWindow()
{
	// 홀드 방어 GE: Target에 State.Combat.Parrying 부여(50% 감소).
	// 버튼 떼는 시점 = 끝이라 시간 고정 불가 -> Infinite. GA_Parry가 OnCleanup에서 제거.
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	UTargetTagsGameplayEffectComponent* TagsComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTagsComp"));
	FInheritedTagContainer InheritableTags;
	InheritableTags.Added.AddTag(GameplayTags::State_Combat_Parrying);
	TagsComp->SetAndApplyTargetTagChanges(InheritableTags);
	GEComponents.Add(TagsComp);
}
