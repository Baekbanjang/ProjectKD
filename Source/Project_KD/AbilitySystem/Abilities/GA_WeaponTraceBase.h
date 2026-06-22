#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_ActionBase.h"
#include "AbilitySystem/Tasks/AT_WeaponTrace.h"
#include "GA_WeaponTraceBase.generated.h"

class UAnimMontage;
class UAT_WeaponTrace;
class UGameplayEffect;

// Melee weapon-trace base (player LightAttack, enemy attacks). Plays a montage, runs AT_WeaponTrace on
// TraceBegin/End notifies, applies DamageEffectClass to hit ASCs. Hooks: OnActivated / OnTargetHit / OnCleanup.
UCLASS(Abstract)
class PROJECT_KD_API UGA_WeaponTraceBase : public UGA_ActionBase
{
	GENERATED_BODY()

public:
	UGA_WeaponTraceBase();

protected:
	// Engine (ASC) calls this via the base pointer — keep base's protected visibility.
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName WeaponMeshComponentTag = TEXT("Weapon");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName StartSocket = TEXT("Spear_Bottom");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName EndSocket = TEXT("Spear_Tip");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	ETraceMode TraceMode = ETraceMode::TipLine;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon", meta = (ClampMin = "0.1"))
	float CapsuleRadius = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	bool bOncePerActor = true;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Debug")
	bool bDrawDebug = false;

	// Hook after montage/trace setup (e.g. combo window).
	virtual void OnActivated();
	virtual float GetEffectiveMontagePlayRate() const { return MontagePlayRate;}

	// Per-unique-hit-actor hook (post-damage). Hit carries impact for contact-point cues.
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) {}

	// Override must call Super (chains trace cleanup).
	virtual void OnCleanup(bool bWasCancelled) override;

private:
	UFUNCTION() void OnTraceBeginEvent(FGameplayEventData Payload);
	UFUNCTION() void OnTraceEndEvent(FGameplayEventData Payload);
	UFUNCTION() void OnWeaponHit(const FHitResult& Hit);
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();

	UPROPERTY()
	TObjectPtr<UAT_WeaponTrace> ActiveTraceTask;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
};
