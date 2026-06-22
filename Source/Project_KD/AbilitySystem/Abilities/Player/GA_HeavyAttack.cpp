// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_HeavyAttack.h"

#include "KDGameplayTags.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"

UGA_HeavyAttack::UGA_HeavyAttack()
{
	ComboInputTag = GameplayTags::Input_Combo_Heavy;
}

void UGA_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!UKDAbilityStatics::TryConsumeStamina(ASC, StaminaCostGE, StaminaRegenBlockGE, HeavyStaminaCost))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // 부족 → 강공 발동 차단
		return;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
