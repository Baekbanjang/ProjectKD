#include "AbilitySystem/Abilities/GA_ActionBase.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "Combat/LockOnComponent.h"

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

FActiveGameplayEffectHandle UGA_ActionBase::ApplySelfEffect(TSubclassOf<UGameplayEffect> GEClass)
{
	// 기능 : 자기 자신에게 GE 적용
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !GEClass) return FActiveGameplayEffectHandle();

	const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GEClass, 1.0f, Ctx);
	if (!Spec.IsValid()) return FActiveGameplayEffectHandle();

	return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
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

ULockOnComponent* UGA_ActionBase::GetLockOnComponentFromActorInfo() const
{
	// 기능 : 아바타에서 락온 컴포넌트 조회
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<ULockOnComponent>() : nullptr;
}

UComboComponent* UGA_ActionBase::GetComboComponentFromActorInfo() const
{
	// 기능 : 아바타에서 콤보 컴포넌트 조회
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<UComboComponent>() : nullptr;
}

AActor* UGA_ActionBase::FindAutoAimTarget(float Range, float ConeAngle) const
{
	// 기능 : 자동 조준 대상 1명 반환
	ULockOnComponent* LockOn = GetLockOnComponentFromActorInfo();
	if (!LockOn) return nullptr;
	
	return LockOn->IsLockedOn() ? LockOn->GetLockedTarget() : LockOn->FindBestTarget(Range, ConeAngle);
}
