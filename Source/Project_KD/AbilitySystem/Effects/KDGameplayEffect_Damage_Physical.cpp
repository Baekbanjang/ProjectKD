#include "AbilitySystem/Effects/KDGameplayEffect_Damage_Physical.h"

#include "AbilitySystem/Attributes/KDCombatAttributeSet.h"
#include "KDGameplayTags.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UKDGameplayEffect_Damage_Physical::UKDGameplayEffect_Damage_Physical()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Route through the IncomingDamage gateway (AS_Combat::PostGameplayEffectExecute drains it,
	// applies Defense mitigation, then subtracts from Health). Positive magnitude = damage dealt.
	FGameplayModifierInfo DamageMod;
	DamageMod.Attribute = UKDCombatAttributeSet::GetIncomingDamageAttribute();
	DamageMod.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = GameplayTags::SetByCaller_AttackPower;
	DamageMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(DamageMod);

	// i-frame 차단: Invulnerable/Dead 보유 시 자동 reject.
	// CDO 생성자에서 AddComponent<>() 직접 호출 금지(UE5.6 fatal) → CreateDefaultSubobject + GEComponents.Add.
	UTargetTagRequirementsGameplayEffectComponent* ReqComp =
		CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("TagRequirementsComp"));
	ReqComp->ApplicationTagRequirements.IgnoreTags.AddTag(GameplayTags::State_Combat_Invulnerable);
	// Dead targets reject damage entirely (no modifiers, no hit cue/boneshake) — corpse stops being hit.
	ReqComp->ApplicationTagRequirements.IgnoreTags.AddTag(GameplayTags::State_Dead);
	GEComponents.Add(ReqComp);
}
