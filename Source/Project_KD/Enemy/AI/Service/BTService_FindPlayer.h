// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_FindPlayer.generated.h"

// Controller perception 결과(시야+피격, 상실 후 기억 포함)를 BB로 미러 — TargetActor 세팅/클리어 + AttackRange 안이면 bCanAttack.
UCLASS()
class PROJECT_KD_API UBTService_FindPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// TargetActor (Object) key — assigned in the BT editor.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// bCanAttack (Bool) key — set true when the player is within the pawn's AttackRange.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanAttackKey;
};
