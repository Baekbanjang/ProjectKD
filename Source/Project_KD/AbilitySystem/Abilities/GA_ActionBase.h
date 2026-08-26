#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ActionBase.generated.h"

class UKDLockOnComponent;
class UKDComboComponent;

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

	// 자기 자신에게 GE 적용 — 상태 GE(무적·블록·패링)용. 실패 시 무효 핸들
	FActiveGameplayEffectHandle ApplySelfEffect(TSubclassOf<UGameplayEffect> GEClass);

	UPROPERTY(EditDefaultsOnly, Category = "Action|Safety", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float MaxDurationSafetyMult = 1.5f;

	// 아바타에서 컴포넌트 조회
	UFUNCTION(BlueprintPure, Category = "Ability")
	UKDLockOnComponent* GetLockOnComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	UKDComboComponent* GetComboComponentFromActorInfo() const;

	// 자동 조준 대상 — 락온 중이면 고정 타겟 / 그 외 범위 안 각도 최소 적
	AActor* FindAutoAimTarget(float Range, float ConeAngle) const;

private:
	void OnSafetyTimeout();

	FTimerHandle SafetyTimerHandle;
};
