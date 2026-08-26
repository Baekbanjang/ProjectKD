// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/KDAnimNotify_Shoot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KDGameplayTags.h"

void UKDAnimNotify_Shoot::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                       const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!IsValid(MeshComp)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	// ASC 없는 애님 에디터 프리뷰 액터 제외
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;
	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::Event_Montage_Shoot;
	Payload.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Payload.EventTag, Payload);
}

FString UKDAnimNotify_Shoot::GetNotifyName_Implementation() const
{
	return TEXT("Shoot");
}
