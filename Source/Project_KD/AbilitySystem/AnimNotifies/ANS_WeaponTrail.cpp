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
	
	UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystem, AttachMesh, SocketName,
			LocationOffset, RotationOffset,
			EAttachLocation::SnapToTarget, true);
	if (!IsValid(Trail)) return;
	
	Trail->SetVariableFloat(TEXT("SwordLength"), SwordLength);   // 무기 길이
	Trail->SetVariableFloat(TEXT("TrailWidth"), TrailWidth);     // 트레일 폭

	//  NotifyEnd 없이 사라진 경우(액터 파괴 / 레벨 전환) 대비
	for (auto It = SpawnedTrails.CreateIterator(); It; ++It)
	{
		if (!It->Key.IsValid()) It.RemoveCurrent();
	}
	SpawnedTrails.Add(MeshComp, Trail);   // 액터별 등록 
}

void UANS_WeaponTrail::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	TWeakObjectPtr<UNiagaraComponent> Found;
	if (SpawnedTrails.RemoveAndCopyValue(MeshComp, Found))
	{
		if (UNiagaraComponent* Trail = Found.Get())
		{
			Trail->Deactivate();
		}
	}
}

FString UANS_WeaponTrail::GetNotifyName_Implementation() const
{
	return TEXT("WeaponTrail");
}
