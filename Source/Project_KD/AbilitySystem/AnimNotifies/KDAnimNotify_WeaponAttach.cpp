// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/KDAnimNotify_WeaponAttach.h"

#include "Combat/KDWeaponComponent.h"

void UKDAnimNotify_WeaponAttach::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// 메시 소유자에서 무기 컴포넌트를 찾아 재부여
	TInlineComponentArray<UKDWeaponComponent*> WeaponComps(Owner);

	for (UKDWeaponComponent* WeaponComp : WeaponComps)
	{
		if (!TargetWeaponTag.IsNone() && WeaponComp->GetWeaponComponentTag() != TargetWeaponTag) continue;
		
		if (Target == EWeaponAttachTarget::ToHand)
		{
			WeaponComp->AttachWeaponToHand();
		}
		else
		{
			WeaponComp->AttachWeaponToSheath();
		}
	}
}

FString UKDAnimNotify_WeaponAttach::GetNotifyName_Implementation() const
{
	return TEXT("Weapon Attach");
}
