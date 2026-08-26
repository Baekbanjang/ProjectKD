// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerAttackBase.h"
#include "AbilitySystem/Combo/KDComboTreeDataAsset.h"


void UGA_PlayerAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	// 콤보 컴포넌트가 없으면(= 플레이어가 아니면) 노드도 없음 -> 몽타주 없이 종료
	ApplyComboNode(ComboInputTag, EComboContext::Ground,
		DefaultDamageEffectClass, DefaultDamageMultiplier, DefaultKnockbackMultiplier, DefaultPoiseMultiplier);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
