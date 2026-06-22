// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_SelectAttack.generated.h"

// 공격 고르기 노드. 적 종류마다 행동트리를 따로 만들지 않으려고, 적 데이터(EnemyDefinition)의 공격 중
// 사거리 안 + 지금 쓸 수 있는 것만 추려 가중치 랜덤으로 하나 골라 태그를 블랙보드에 씀 → ActivateAbilityByTag가 발동.
UCLASS()
class PROJECT_KD_API UBTTask_SelectAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 거리 기준 타겟(읽기) — FindPlayer가 세팅한 TargetActor 키를 가리키게 BP에서 연결.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 선택된 공격 어빌리티 태그의 Name(쓰기) — ActivateAbilityByTag가 같은 키를 읽음.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SelectedAbilityTagNameKey;
};
