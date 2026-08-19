#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_ActionBase.h"
#include "AbilitySystem/Tasks/AT_MeleeTrace.h"
#include "GA_MeleeTraceBase.generated.h"

class UANS_MeleeTrace;
class UAnimMontage;
class UAT_MeleeTrace;
class UGameplayEffect;

// Melee weapon-trace base (player LightAttack, enemy attacks). Plays a montage, runs AT_MeleeTrace on
// TraceBegin/End notifies, applies DamageEffectClass to hit ASCs. Hooks: OnActivated / OnTargetHit / OnCleanup.
UCLASS(Abstract)
class PROJECT_KD_API UGA_MeleeTraceBase : public UGA_ActionBase
{
	GENERATED_BODY()

public:
	UGA_MeleeTraceBase();

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

	// 해당 공격의 데미지 배수 - 콤보 노드 없을 시
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName WeaponMeshComponentTag = TEXT("Sword");
	
	// 판정 출처 기본값
	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	ETraceMeshSource MeshSource = ETraceMeshSource::Weapon;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName StartSocket = TEXT("Sword_Bottom");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName EndSocket = TEXT("Sword_Tip");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	ETraceMode TraceMode = ETraceMode::TipLine;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon", meta = (ClampMin = "0.1"))
	float CapsuleRadius = 3.0f;

	// 판정창 1개당 액터 1히트 - 검이 닿아 있는 동안 매 프레임 반복 타격 차단
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

	const UANS_MeleeTrace* GetActiveWindow() const { return ActiveWindow.Get(); }

private:
	UFUNCTION() void OnTraceBeginEvent(FGameplayEventData Payload);
	UFUNCTION() void OnTraceEndEvent(FGameplayEventData Payload);
	UFUNCTION() void OnWeaponHit(const FHitResult& Hit);
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();

	UPROPERTY()
	TObjectPtr<UAT_MeleeTrace> ActiveTraceTask;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
	
	TWeakObjectPtr<const UANS_MeleeTrace> ActiveWindow;
};
