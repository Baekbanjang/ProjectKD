#include "AbilitySystem/AnimNotifies/ANS_WeaponTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KDGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UANS_WeaponTrace::UANS_WeaponTrace()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif
}

void UANS_WeaponTrace::NotifyBegin(
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

void UANS_WeaponTrace::NotifyEnd(
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

FString UANS_WeaponTrace::GetNotifyName_Implementation() const
{
	return TEXT("WeaponTrace");
}
