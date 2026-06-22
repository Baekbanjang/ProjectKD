#include "AbilitySystem/AnimNotifies/ANS_CancelWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UANS_CancelWindow::UANS_CancelWindow()
	: CancelTag(GameplayTags::State_Combat_CanCancel)
{
}

void UANS_CancelWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !CancelTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->AddLooseGameplayTag(CancelTag);
}

void UANS_CancelWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !CancelTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(CancelTag);
}

FString UANS_CancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("CancelWindow");
}
