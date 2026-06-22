// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Task/BTTask_ActivateAbilityByTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_ActivateAbilityByTag::UBTTask_ActivateAbilityByTag()
{
	NodeName = TEXT("Activate Ability By Tag");
	bNotifyTick = true;

	// 선택 키는 Name 타입만 — SelectAttack이 쓴 어빌리티 태그 Name을 읽는다.
	AbilityTagNameKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ActivateAbilityByTag, AbilityTagNameKey));
}

void UBTTask_ActivateAbilityByTag::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		AbilityTagNameKey.ResolveSelectedKey(*BBAsset);
	}
}

// BB 선택 키가 유효 Name을 주면 그 태그, 아니면 고정 ActivationTag로 폴백. 파일 로컬 헬퍼(헤더 = 오너 영역).
static FGameplayTag GetEffectiveTag(UBehaviorTreeComponent& OwnerComp, const FBlackboardKeySelector& TagNameKey, const FGameplayTag& FallbackTag)
{
	if (TagNameKey.IsSet())
	{
		if (const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		{
			const FName TagName = BB->GetValueAsName(TagNameKey.SelectedKeyName);
			if (!TagName.IsNone())
			{
				const FGameplayTag Resolved = FGameplayTag::RequestGameplayTag(TagName, false);
				if (Resolved.IsValid()) return Resolved;
			}
		}
	}
	return FallbackTag;
}

// 태그 매칭 — 태그 공유 spec을 구별 못 하지만, 적 공격 태그는 GA와 1:1 매핑이라 충분.
static bool IsAbilityActiveByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag)
{
	FGameplayTagContainer Query;
	Query.AddTag(Tag);
	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Query, Specs);
	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if (Spec && Spec->IsActive()) return true;
	}
	return false;
}

// owner pawn의 ASC. 파일 로컬 헬퍼.
static UAbilitySystemComponent* GetPawnASC(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	return Pawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn) : nullptr;
}

EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAbilitySystemComponent* ASC = GetPawnASC(OwnerComp);
	const FGameplayTag EffectiveTag = GetEffectiveTag(OwnerComp, AbilityTagNameKey, ActivationTag);
	if (!ASC || !EffectiveTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTagContainer Tags;
	Tags.AddTag(EffectiveTag);
	if (!ASC->TryActivateAbilitiesByTag(Tags))
	{
		return EBTNodeResult::Failed;
	}

	// 즉시 종료 어빌리티(몽타주 없음)는 TryActivate 안에서 동기 종료 → InProgress 대신 Succeeded.
	// 로컬 ASC는 동기 활성화라 same-frame race 없음.
	return IsAbilityActiveByTag(ASC, EffectiveTag) ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
}

void UBTTask_ActivateAbilityByTag::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	UAbilitySystemComponent* ASC = GetPawnASC(OwnerComp);
	const FGameplayTag EffectiveTag = GetEffectiveTag(OwnerComp, AbilityTagNameKey, ActivationTag);
	if (!ASC || !EffectiveTag.IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// ExecuteTask confirmed active before InProgress, so a now-inactive spec means it genuinely ended.
	if (!IsAbilityActiveByTag(ASC, EffectiveTag))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
