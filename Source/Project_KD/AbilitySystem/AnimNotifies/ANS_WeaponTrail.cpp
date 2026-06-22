#include "AbilitySystem/AnimNotifies/ANS_WeaponTrail.h"

#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void UANS_WeaponTrail::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp) || !IsValid(NiagaraSystem)) return;

	SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem, MeshComp, SocketName,
		LocationOffset, RotationOffset,
		EAttachLocation::SnapToTarget, true);

	if (!IsValid(SpawnedComponent)) return;

	SpawnedComponent->SetVariableFloat(TEXT("SwordLength"), SwordLength);
	SpawnedComponent->SetVariableFloat(TEXT("TrailWidth"), TrailWidth);
}

void UANS_WeaponTrail::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (IsValid(SpawnedComponent))
		SpawnedComponent->Deactivate(); // Stop emission, let existing ribbon fade out
	SpawnedComponent = nullptr;
}

FString UANS_WeaponTrail::GetNotifyName_Implementation() const
{
	return TEXT("WeaponTrail");
}
