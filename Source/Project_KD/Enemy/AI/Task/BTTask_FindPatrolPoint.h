// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_FindPatrolPoint.generated.h"

// 홈 기준 PatrolRadius 안 navmesh 도달가능 랜덤점을 BB Vector에 씀(MoveTo 소비).
// 반경은 캐릭터(GetPatrolRadius) 소유, 0이면 Failed(패트롤 안 하는 적).
UCLASS()
class PROJECT_KD_API UBTTask_FindPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindPatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 패트롤 목표 위치(쓰기, Vector) — 스톡 MoveTo 노드가 이 키를 소비.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector OutputLocationKey;
};
