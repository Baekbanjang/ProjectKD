// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_MovementCancel.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"

void UKDAnimNotifyState_MovementCancel::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!ASC) return;

	ASC->AddLooseGameplayTag(GameplayTags::State_Combat_MovementCanCancel);
}

void UKDAnimNotifyState_MovementCancel::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(GameplayTags::State_Combat_MovementCanCancel);
}
