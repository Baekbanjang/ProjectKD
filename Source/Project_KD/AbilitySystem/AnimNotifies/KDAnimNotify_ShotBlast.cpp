// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/KDAnimNotify_ShotBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KDGameplayTags.h"

void UKDAnimNotify_ShotBlast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
	// 기능 : 발사 프레임에 GA 트리거용 이벤트 전송
	Super::Notify(MeshComp, Animation, EventReference);
	if (!IsValid(MeshComp)) return;
	
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;
	
	// ASC 없는 애님 에디터 프리뷰 액터 제외
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;
	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::Event_Montage_ShotBlast;
	Payload.Instigator = Owner;
	Payload.OptionalObject = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Payload.EventTag, Payload);
}

FString UKDAnimNotify_ShotBlast::GetNotifyName_Implementation() const
{
	return TEXT("ShotBlast");
}
