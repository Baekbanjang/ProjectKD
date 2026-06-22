#include "AbilitySystem/AnimNotifies/ANS_TelegraphWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UANS_TelegraphWindow::UANS_TelegraphWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Orange;
#endif
}

void UANS_TelegraphWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

void UANS_TelegraphWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

FString UANS_TelegraphWindow::GetNotifyName_Implementation() const
{
	return TEXT("TelegraphWindow");
}
