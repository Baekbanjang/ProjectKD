#include "AbilitySystem/Abilities/KDGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AbilitySystem/Combo/KDComboComponent.h"
#include "Combat/Data/KDTargetFilter.h"
#include "Combat/KDLockOnComponent.h"

UKDGameplayAbility::UKDGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	// bRetriggerInstancedAbility per-subclass: Dodge=true (SB-tone cancel-friendly),
	// LightAttack/EnemyAttack=false (preserve combo-window buffer pattern).
}

void UKDGameplayAbility::EndAbility(
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

void UKDGameplayAbility::StartSafetyTimer(float MontagePlayLength, float PlayRate)
{
	if (UWorld* World = GetWorld())
	{
		const float SafeRate = FMath::Max(PlayRate, 0.1f);
		const float Duration = (MontagePlayLength / SafeRate) * MaxDurationSafetyMult;
		World->GetTimerManager().SetTimer(SafetyTimerHandle, this,
			&UKDGameplayAbility::OnSafetyTimeout, Duration, false);
	}
}

void UKDGameplayAbility::ClearSafetyTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SafetyTimerHandle);
	}
}

FActiveGameplayEffectHandle UKDGameplayAbility::ApplySelfEffect(TSubclassOf<UGameplayEffect> GEClass)
{
	// 기능 : 자기 자신에게 GE 적용
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !GEClass) return FActiveGameplayEffectHandle();

	const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1.0f, Ctx);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();

	return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UKDGameplayAbility::OnSafetyTimeout()
{
	if (!IsActive())
	{
		return;
	}
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
		GetCurrentActivationInfo(), false, true);
}

UKDLockOnComponent* UKDGameplayAbility::GetLockOnComponentFromActorInfo() const
{
	// 기능 : 아바타에서 락온 컴포넌트 조회
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<UKDLockOnComponent>() : nullptr;
}

UKDComboComponent* UKDGameplayAbility::GetComboComponentFromActorInfo() const
{
	// 기능 : 아바타에서 콤보 컴포넌트 조회
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<UKDComboComponent>() : nullptr;
}

AActor* UKDGameplayAbility::FindAutoAimTarget(const FKDTargetFilter& Filter) const
{
	// 기능 : 자동 조준 대상 1명 반환
	UKDLockOnComponent* LockOn = GetLockOnComponentFromActorInfo();
	if (!LockOn) return nullptr;
	
	return LockOn->IsLockedOn() ? LockOn->GetLockedTarget() : LockOn->FindTargetByFilter(Filter);
}
