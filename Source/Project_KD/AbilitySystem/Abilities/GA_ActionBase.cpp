#include "AbilitySystem/Abilities/GA_ActionBase.h"

#include "Engine/World.h"
#include "TimerManager.h"

UGA_ActionBase::UGA_ActionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	// bRetriggerInstancedAbility per-subclass: Dodge=true (SB-tone cancel-friendly),
	// LightAttack/EnemyAttack=false (preserve combo-window buffer pattern).
}

void UGA_ActionBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearSafetyTimer();
	OnCleanup(bWasCancelled);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ActionBase::StartSafetyTimer(float MontagePlayLength, float PlayRate)
{
	if (UWorld* World = GetWorld())
	{
		const float SafeRate = FMath::Max(PlayRate, 0.1f);
		const float Duration = (MontagePlayLength / SafeRate) * MaxDurationSafetyMult;
		World->GetTimerManager().SetTimer(SafetyTimerHandle, this,
			&UGA_ActionBase::OnSafetyTimeout, Duration, false);
	}
}

void UGA_ActionBase::ClearSafetyTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SafetyTimerHandle);
	}
}

void UGA_ActionBase::OnSafetyTimeout()
{
	if (!IsActive())
	{
		return;
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
		GetCurrentActivationInfo(), false, true);
}
