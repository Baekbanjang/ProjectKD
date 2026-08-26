// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_EnemyAttackWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"

UKDAnimNotifyState_EnemyAttackWindow::UKDAnimNotifyState_EnemyAttackWindow()
: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Blue; 
#endif
}

void UKDAnimNotifyState_EnemyAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !AttackWindowTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->AddLooseGameplayTag(AttackWindowTag);
}

void UKDAnimNotifyState_EnemyAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !AttackWindowTag.IsValid()) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(AttackWindowTag);
}

FString UKDAnimNotifyState_EnemyAttackWindow::GetNotifyName_Implementation() const
{
	return TEXT("EnemyAttackWindow");
}
