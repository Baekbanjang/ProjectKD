#include "AbilitySystem/GameplayCues/GCN_EnemyTelegraph.h"

#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

bool AGCN_EnemyTelegraph::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);

	if (!IsValid(MyTarget))
	{
		return false;
	}

	// Resolve which telegraph fired. OriginalTag = the leaf passed to AddGameplayCue
	// (.Parryable/.Unblockable/.Lethal). MatchedTagName may be the parent this cue is
	// registered on, so prefer OriginalTag and fall back only if it is empty.
	const FGameplayTag Key = Parameters.OriginalTag.IsValid()
		? Parameters.OriginalTag
		: Parameters.MatchedTagName;

	const TSoftObjectPtr<UNiagaraSystem>* Found = TelegraphFXByTag.Find(Key);
	if (!Found)
	{
		return false;
	}

	UNiagaraSystem* FX = Found->LoadSynchronous();
	if (!IsValid(FX))
	{
		return false;
	}

	// Attach to the weapon mesh (glowing blade) — same component the trace uses, so the
	// tell reads as "this weapon is winding up". Fall back to the actor root if absent.
	USkeletalMeshComponent* WeaponMesh = nullptr;
	TArray<UActorComponent*> Components;
	MyTarget->GetComponents(USkeletalMeshComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (Comp->ComponentHasTag(WeaponMeshComponentTag))
		{
			WeaponMesh = Cast<USkeletalMeshComponent>(Comp);
			break;
		}
	}

	USceneComponent* AttachTo = WeaponMesh ? static_cast<USceneComponent*>(WeaponMesh) : MyTarget->GetRootComponent();
	if (!IsValid(AttachTo))
	{
		return false;
	}

	// bAutoDestroy=true: looping FX is stopped by Deactivate in OnRemove, then self-destroys
	// once the trailing particles finish (matches ANS_WeaponTrail lifetime handling).
	ActiveFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		FX, AttachTo, WeaponSocketName,
		FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget, true);

	if (IsValid(ActiveFX) && !FMath::IsNearlyEqual(TelegraphScale, 1.0f))
	{
		ActiveFX->SetRelativeScale3D(FVector(TelegraphScale));
	}

	return true;
}

bool AGCN_EnemyTelegraph::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// Stop emission; let existing particles fade. This instance is pooled and reused, so
	// clearing ActiveFX is required to avoid stopping a later activation's FX.
	if (IsValid(ActiveFX))
	{
		ActiveFX->Deactivate();
	}
	ActiveFX = nullptr;

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
