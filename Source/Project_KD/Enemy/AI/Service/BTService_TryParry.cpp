// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Service/BTService_TryParry.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "KDGameplayTags.h"

UBTService_TryParry::UBTService_TryParry()
{
	NodeName = TEXT("Try Parry");
	Interval = 0.15f;
	RandomDeviation = 0.02f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_TryParry, TargetActorKey), AActor::StaticClass());
}

void UBTService_TryParry::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_TryParry::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!ParryAbilityTag.IsValid()) return;

	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return;

	APawn* Enemy = AICon->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Enemy || !Target) return;

	// 근접 게이트 — ParryRange 밖이면 패링 안 함.
	if (FVector::DistSquared(Enemy->GetActorLocation(), Target->GetActorLocation()) > ParryRange * ParryRange) return;

	// 리액티브 — 플레이어가 공격 중일 때만 반응. State.Combat.Attacking은 플레이어 공격 GA가 부여.
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking)) return;

	// 확률 롤 — 매번 막지 않게(빈도 = Interval × ParryChance).
	if (FMath::FRand() > ParryChance) return;

	// 적 ASC에 패링 GA 직접 발동 — 현재 BT 흐름을 인터럽트하는 리액티브 반응.
	// 어빌리티 미부여 변형은 그냥 실패(탈부착 자동 게이트). 쿨다운/공격중 차단은 GA 소관.
	if (UAbilitySystemComponent* EnemyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy))
	{
		EnemyASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ParryAbilityTag));
	}
}
