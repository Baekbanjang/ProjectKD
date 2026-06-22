// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Library/KDAbilityStatics.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Attributes/AS_Player.h"

bool UKDAbilityStatics::TryConsumeStamina(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> StaminaCostGE,
                                          TSubclassOf<UGameplayEffect> StaminaRegenBlockGE, float Cost)
{
	if (Cost <= 0.f) return true;
	if (!IsValid(ASC)) return true; // stamina 없는 액터는 통과

	// 현재 스태미나 사용 비용보다 적으면 false
	if (ASC->GetNumericAttribute(UAS_Player::GetStaminaAttribute()) < Cost) return false;

	// 스태미나 소모
	if (StaminaCostGE)
	{
		const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostGE, 1.0f, Ctx);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(GameplayTags::SetByCaller_Stamina, -Cost);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// 회복 차단 스태미나(스태미나 소모 직후 1초간 회복 차단)
	if (StaminaRegenBlockGE)
	{
		const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaRegenBlockGE, 1.0f, Ctx);
		if (Spec.IsValid()) ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
	return true;
}
