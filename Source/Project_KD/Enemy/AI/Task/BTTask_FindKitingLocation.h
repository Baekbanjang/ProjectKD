// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_FindKitingLocation.generated.h"

// 적이 플레이어와 거리를 두려고 뒤로 물러설 위치를 정하는 노드. StandoffRange가 0인 근접형은 그냥 실패.
// 플레이어가 StandoffRange×TriggerRatio보다 가까우면 반대 방향 지점을 블랙보드에 써서 MoveTo가 그쪽으로 이동.
UCLASS()
class PROJECT_KD_API UBTTask_FindKitingLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindKitingLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 후퇴를 시작하는 거리 = StandoffRange × 이 비율. 1보다 작게(시작 거리를 목표보다 짧게 둬서 경계에서 안 떨게).
	UPROPERTY(EditAnywhere, Category = "Kiting", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float TriggerRatio = 0.85f;

	// 플레이어가 이보다 가까우면 천천히 물러서지 않고 빠른 백스텝으로 넘어감. 0이면 끔. TriggerRatio 거리보다 작게.
	UPROPERTY(EditAnywhere, Category = "Kiting", meta = (ClampMin = "0.0"))
	float DangerRange = 250.f;

	// 거리 기준이 되는 타겟(읽기) — FindPlayer가 세팅한 TargetActor 키를 그대로 가리키게 BP에서 연결.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 후퇴 목표 위치(쓰기, Vector) — 스톡 MoveTo 노드가 이 키를 소비.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector OutputLocationKey;
};
