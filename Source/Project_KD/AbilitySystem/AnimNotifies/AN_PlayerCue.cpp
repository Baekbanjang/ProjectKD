// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/AN_PlayerCue.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAN_PlayerCue::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !CueTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;
	
	FGameplayCueParameters CueParams;
	CueParams.Instigator = Owner;
	ASC->ExecuteGameplayCue(CueTag, CueParams);
}

FString UAN_PlayerCue::GetNotifyName_Implementation() const
{
	return TEXT("GameplayCue");
}
