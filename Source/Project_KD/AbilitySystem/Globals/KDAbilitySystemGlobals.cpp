#include "AbilitySystem/Globals/KDAbilitySystemGlobals.h"
#include "AbilitySystem/Context/KDGameplayEffectContext.h"

FGameplayEffectContext* UKDAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// 기능 : MakeEffectContext 가 KDGameplayEffectContext 타입으로 생성
	return new FKDGameplayEffectContext();
}
