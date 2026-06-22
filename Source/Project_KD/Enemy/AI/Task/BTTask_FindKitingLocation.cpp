// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Task/BTTask_FindKitingLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

#include "Enemy/KDEnemyBaseCharacter.h"

UBTTask_FindKitingLocation::UBTTask_FindKitingLocation()
{
	NodeName = TEXT("Find Kiting Location");

	// Restrict the editor key pickers to their expected types.
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindKitingLocation, TargetActorKey), AActor::StaticClass());
	OutputLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindKitingLocation, OutputLocationKey));
}

void UBTTask_FindKitingLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
		OutputLocationKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_FindKitingLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return EBTNodeResult::Failed;

	AKDEnemyBaseCharacter* SelfPawn = Cast<AKDEnemyBaseCharacter>(AICon->GetPawn());
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!SelfPawn || !Target)
	{
		return EBTNodeResult::Failed;
	}

	// StandoffRange = 적이 유지하려는 거리(캐릭터가 가짐). 0이면 근접형이라 후퇴 안 함 → 실패(공격 쪽으로 넘어감).
	const float StandoffRange = SelfPawn->GetStandoffRange();
	if (StandoffRange <= 0.f) return EBTNodeResult::Failed;

	const FVector SelfLoc = SelfPawn->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();

	// 바닥 기준 거리(높이 무시). 이미 충분히 멀면(StandoffRange × TriggerRatio 이상) 후퇴할 필요 없음.
	const float Dist = FVector::Dist2D(SelfLoc, TargetLoc);
	if (Dist >= StandoffRange * TriggerRatio)
	{
		return EBTNodeResult::Failed;
	}

	// 너무 가까우면(DangerRange 안) 천천히 뒷걸음 대신 실패 → 행동트리가 빠른 백스텝으로 넘어감.
	if (Dist < DangerRange) return EBTNodeResult::Failed;

	// 플레이어 반대 방향(바닥). 완전히 겹쳐서 방향이 안 나오면 적의 뒤쪽으로.
	FVector AwayDir = SelfLoc - TargetLoc;
	AwayDir.Z = 0.f;
	if (!AwayDir.Normalize())
	{
		AwayDir = -SelfPawn->GetActorForwardVector();
		AwayDir.Z = 0.f;
		AwayDir.Normalize();
	}

	// 플레이어에서 StandoffRange만큼 떨어진 지점. 높이는 적 현재 높이로.
	FVector Standoff = TargetLoc + AwayDir * StandoffRange;
	Standoff.Z = SelfLoc.Z;

	// 그 지점이 실제로 걸어갈 수 있는 바닥인지 확인 — 벽 구석처럼 못 가는 곳이면 실패시켜 다른 행동으로.
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(SelfPawn->GetWorld());
	FNavLocation Projected;
	if (!NavSys || !NavSys->ProjectPointToNavigation(Standoff, Projected))
	{
		return EBTNodeResult::Failed;
	}

	BB->SetValueAsVector(OutputLocationKey.SelectedKeyName, Projected.Location);
	return EBTNodeResult::Succeeded;
}
