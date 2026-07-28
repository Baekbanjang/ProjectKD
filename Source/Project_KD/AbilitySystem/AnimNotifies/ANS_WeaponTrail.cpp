#include "AbilitySystem/AnimNotifies/ANS_WeaponTrail.h"

#include "Components/MeshComponent.h"
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

	// 붙일 메시 결정 — 태그가 있으면 무기 메시, 없으면 캐릭터 본체
	UMeshComponent* AttachMesh = MeshComp;
	if (WeaponMeshComponentTag != NAME_None)
	{
		AttachMesh = nullptr;
		if (AActor* Owner = MeshComp->GetOwner())
		{
			TArray<UActorComponent*> Components;
			Owner->GetComponents(UMeshComponent::StaticClass(), Components);
			for (UActorComponent* Comp : Components)
			{
				if (Comp->ComponentHasTag(WeaponMeshComponentTag))
				{
					AttachMesh = Cast<UMeshComponent>(Comp);   // UMeshComponent (스태틱 무기)
					break;
				}
			}
		}
	}

	if (!IsValid(AttachMesh)) return;
	SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystem, AttachMesh, SocketName,
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
