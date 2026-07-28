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

	// 트레일을 붙일 무기 메시의 ComponentTag. 비우면 캐릭터 본체 메시(맨손/발차기용)
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName WeaponMeshComponentTag = TEXT("Sword");
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName SocketName = TEXT("Sword_Bottom");

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
