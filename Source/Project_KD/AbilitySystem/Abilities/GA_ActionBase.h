#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ActionBase.generated.h"

// Common base for all action GAs (LightAttack, Dodge, EnemyAttack).
// Centralizes SafetyTimer (montage length / play rate × mult) and EndAbility → OnCleanup pipeline.
// bRetriggerInstancedAbility: Dodge=true (cancel-friendly), Attack=false (combo buffer).
UCLASS(Abstract)
class PROJECT_KD_API UGA_ActionBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ActionBase();

protected:
	// Engine (ASC) calls this via the base pointer — keep base's protected visibility.
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	// Subclass releases its own resources here. SafetyTimer + EndAbility chain are handled
	// by base — do not re-call EndAbility from this hook.
	virtual void OnCleanup(bool bWasCancelled) {}

	void StartSafetyTimer(float MontagePlayLength, float PlayRate);
	void ClearSafetyTimer();

	UPROPERTY(EditDefaultsOnly, Category = "Action|Safety", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float MaxDurationSafetyMult = 1.5f;

private:
	void OnSafetyTimeout();

	FTimerHandle SafetyTimerHandle;
};
