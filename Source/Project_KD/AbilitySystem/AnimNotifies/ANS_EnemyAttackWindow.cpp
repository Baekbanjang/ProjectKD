// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AnimNotifies/ANS_EnemyAttackWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"

UANS_EnemyAttackWindow::UANS_EnemyAttackWindow()
: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Blue; 
#endif
}

void UANS_EnemyAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

void UANS_EnemyAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
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

FString UANS_EnemyAttackWindow::GetNotifyName_Implementation() const
{
	return TEXT("EnemyAttackWindow");
}
