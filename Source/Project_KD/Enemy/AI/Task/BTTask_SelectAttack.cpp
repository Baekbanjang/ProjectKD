// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Task/BTTask_SelectAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

#include "Enemy/EnemyDefinitionDataAsset.h"
#include "Enemy/KDEnemyBaseCharacter.h"

UBTTask_SelectAttack::UBTTask_SelectAttack()
{
	NodeName = TEXT("Select Attack");

	// 에디터 키 픽커를 기대 타입으로 제한.
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SelectAttack, TargetActorKey), AActor::StaticClass());
	SelectedAbilityTagNameKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SelectAttack, SelectedAbilityTagNameKey));
}

void UBTTask_SelectAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
		SelectedAbilityTagNameKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_SelectAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return EBTNodeResult::Failed;

	AKDEnemyBaseCharacter* SelfPawn = Cast<AKDEnemyBaseCharacter>(AICon->GetPawn());
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SelfPawn);
	if (!SelfPawn || !Target || !ASC)
	{
		return EBTNodeResult::Failed;
	}

	const float Dist = FVector::Dist2D(SelfPawn->GetActorLocation(), Target->GetActorLocation());

	// 거리밴드 안 && 지금 발동 가능(쿨다운/코스트/차단태그 OK)인 항목만 후보로. 쿨다운은 GA 자체 GE 관리.
	const TArray<FEnemyAttackEntry>& Entries = SelfPawn->GetAttackEntries();
	TArray<const FEnemyAttackEntry*> Candidates;
	float TotalWeight = 0.f;
	for (const FEnemyAttackEntry& Entry : Entries)
	{
		if (!Entry.AbilityTag.IsValid()) continue;
		if (Dist < Entry.MinRange || Dist > Entry.MaxRange) continue;

		FGameplayTagContainer Query;
		Query.AddTag(Entry.AbilityTag);
		TArray<FGameplayAbilitySpec*> Specs;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Query, Specs);

		bool bCanActivate = false;
		for (const FGameplayAbilitySpec* Spec : Specs)
		{
			if (Spec && Spec->Ability && Spec->Ability->CanActivateAbility(Spec->Handle, ASC->AbilityActorInfo.Get()))
			{
				bCanActivate = true;
				break;
			}
		}
		if (!bCanActivate) continue;

		Candidates.Add(&Entry);
		TotalWeight += FMath::Max(Entry.Weight, 0.f);
	}

	// 후보 0 → Failed로 BT가 reposition 분기.
	if (Candidates.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	// 가중 랜덤. 가중치 합이 0이면(모두 0) 균등 선택으로 폴백.
	const FEnemyAttackEntry* Chosen = nullptr;
	if (TotalWeight > 0.f)
	{
		float Pick = FMath::FRandRange(0.f, TotalWeight);
		for (const FEnemyAttackEntry* Cand : Candidates)
		{
			Pick -= FMath::Max(Cand->Weight, 0.f);
			if (Pick <= 0.f)
			{
				Chosen = Cand;
				break;
			}
		}
	}
	if (!Chosen)
	{
		Chosen = Candidates[FMath::RandHelper(Candidates.Num())];
	}

	// 선택된 어빌리티 태그 Name을 BB에 기록 → ActivateAbilityByTag가 읽어 발동.
	BB->SetValueAsName(SelectedAbilityTagNameKey.SelectedKeyName, Chosen->AbilityTag.GetTagName());
	return EBTNodeResult::Succeeded;
}
