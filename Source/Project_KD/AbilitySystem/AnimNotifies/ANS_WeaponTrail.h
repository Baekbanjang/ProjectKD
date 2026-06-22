#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrail.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// Drag onto Montage timeline to drive the weapon's Niagara trail VFX.
// Begin → spawns the system attached to SocketName and pushes blade dimensions.
// End → deactivates so the ribbon fades out naturally instead of cutting.
UCLASS(meta = (DisplayName = "Weapon Trail"))
class PROJECT_KD_API UANS_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName SocketName = TEXT("weapon_trail");

	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	float SwordLength = 120.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	float TrailWidth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FRotator RotationOffset = FRotator::ZeroRotator;

private:
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedComponent;
};
