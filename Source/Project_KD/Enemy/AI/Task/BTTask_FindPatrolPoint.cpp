// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Task/BTTask_FindPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

#include "Enemy/KDEnemyBaseCharacter.h"

UBTTask_FindPatrolPoint::UBTTask_FindPatrolPoint()
{
	NodeName = TEXT("Find Patrol Point");

	OutputLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindPatrolPoint, OutputLocationKey));
}

void UBTTask_FindPatrolPoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		OutputLocationKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return EBTNodeResult::Failed;

	AKDEnemyBaseCharacter* SelfPawn = Cast<AKDEnemyBaseCharacter>(AICon->GetPawn());
	if (!SelfPawn) return EBTNodeResult::Failed;

	// 반경 0 = 패트롤 안 하는 적 → Failed로 BT가 Idle 분기 폴백.
	const float PatrolRadius = SelfPawn->GetPatrolRadius();
	if (PatrolRadius <= 0.f) return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(SelfPawn->GetWorld());
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	// 홈 기준 도달 가능 지점 — 경로 단절 지점(벽 너머)은 자동 배제. 실패 시(navmesh 밖 홈 등) Failed.
	FNavLocation PatrolPoint;
	if (!NavSys->GetRandomReachablePointInRadius(SelfPawn->GetHomeLocation(), PatrolRadius, PatrolPoint))
	{
		return EBTNodeResult::Failed;
	}

	BB->SetValueAsVector(OutputLocationKey.SelectedKeyName, PatrolPoint.Location);
	return EBTNodeResult::Succeeded;
}
