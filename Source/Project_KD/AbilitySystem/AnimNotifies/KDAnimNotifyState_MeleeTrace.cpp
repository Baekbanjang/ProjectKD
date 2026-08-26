#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_MeleeTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KDGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UKDAnimNotifyState_MeleeTrace::UKDAnimNotifyState_MeleeTrace()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif
}

void UKDAnimNotifyState_MeleeTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;
	// Skip animation editor preview actors that have no ASC.
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::Event_Montage_TraceBegin;
	Payload.Instigator = Owner;
	// Carry this window's trace overrides to the GA. Read synchronously in OnTraceBeginEvent (same call
	// stack), so the notify instance is alive; never stored.
	Payload.OptionalObject = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Payload.EventTag, Payload);
}

void UKDAnimNotifyState_MeleeTrace::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::Event_Montage_TraceEnd;
	Payload.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Payload.EventTag, Payload);
}

FString UKDAnimNotifyState_MeleeTrace::GetNotifyName_Implementation() const
{
	return TEXT("MeleeTrace");
}
