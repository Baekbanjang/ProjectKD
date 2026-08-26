#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_TelegraphWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UKDAnimNotifyState_TelegraphWindow::UKDAnimNotifyState_TelegraphWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Orange;
#endif
}

void UKDAnimNotifyState_TelegraphWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !TelegraphCueTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->AddGameplayCue(TelegraphCueTag);
}

void UKDAnimNotifyState_TelegraphWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !TelegraphCueTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->RemoveGameplayCue(TelegraphCueTag);
}

FString UKDAnimNotifyState_TelegraphWindow::GetNotifyName_Implementation() const
{
	return TEXT("TelegraphWindow");
}
