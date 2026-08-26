#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GameplayTagContainer.h"
#include "KDGameplayCueNotify_EnemyTelegraph.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// One cue for every enemy telegraph. Register GameplayCueTag = "GameplayCue.Enemy.Telegraph"
// (the parent) so .Parryable/.Unblockable/.Lethal all route here. OnActive looks the leaf tag
// up in TelegraphFXByTag and spawns that Niagara FX on the weapon; OnRemove stops it.
// Adding a new telegraph type = one row in TelegraphFXByTag — no new class, no new BP.
UCLASS(Blueprintable)
class PROJECT_KD_API AKDGameplayCueNotify_EnemyTelegraph : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	// Telegraph tag -> looping windup FX. Fill the rows in the BP/CDO details:
	//   GameplayCue.Enemy.Telegraph.Parryable   -> NS_Telegraph_Blue
	//   GameplayCue.Enemy.Telegraph.Unblockable -> NS_Telegraph_Yellow
	//   GameplayCue.Enemy.Telegraph.Lethal      -> NS_Telegraph_Red
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph", meta = (Categories = "GameplayCue.Enemy.Telegraph"))
	TMap<FGameplayTag, TSoftObjectPtr<UNiagaraSystem>> TelegraphFXByTag;

	// Weapon skeletal-mesh component tag (blade glow). Match GA_WeaponTraceBase's WeaponMeshComponentTag.
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FName WeaponMeshComponentTag = TEXT("Weapon");

	// Socket on the weapon to attach FX to. None = weapon root.
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FName WeaponSocketName = NAME_None;

	// Uniform scale applied to the spawned FX (component RelativeScale). 1 = authored size.
	// Works for mesh-renderer / local-space emitters; world-space sprite emitters that ignore
	// component scale need a size User Parameter in the Niagara System instead.
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph", meta = (ClampMin = "0.05"))
	float TelegraphScale = 1.0f;

private:
	// Spawned looping FX, tracked so OnRemove can stop it. Pooled instance -> must clear on remove.
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveFX;
};
