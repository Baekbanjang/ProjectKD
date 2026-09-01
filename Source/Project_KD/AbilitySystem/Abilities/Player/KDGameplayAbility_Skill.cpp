// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_Skill.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Attributes/KDPlayerAttributeSet.h"
#include "AbilitySystem/Effects/KDGameplayEffect_StaminaCost.h"

UKDGameplayAbility_Skill::UKDGameplayAbility_Skill()
{
	CostGameplayEffectClass = UKDGameplayEffect_StaminaCost::StaticClass();
}

bool UKDGameplayAbility_Skill::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 기능 : 잔여 스태미나 직접 비교
	if (StaminaCost <= 0.f)
	{
		return true;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		return false;
	}

	const float Current = ASC->GetNumericAttribute(UKDPlayerAttributeSet::GetStaminaAttribute());
	return Current >= StaminaCost;
}

void UKDGameplayAbility_Skill::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 기능 : 스태미나 사용 후 적용
	if (StaminaCost <= 0.f || !CostGameplayEffectClass)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		Handle, ActorInfo, ActivationInfo, CostGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
	if (!SpecHandle.IsValid())
	{
		return;
	}
	SpecHandle.Data->SetSetByCallerMagnitude(GameplayTags::SetByCaller_Stamina, -StaminaCost);
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}
