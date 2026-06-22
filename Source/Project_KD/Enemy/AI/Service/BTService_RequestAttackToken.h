// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_RequestAttackToken.generated.h"

// FindPlayer 다음 배치. range 안이면 EncounterSubsystem에 토큰 요청 → 성공=bHasAttackToken(공격 진행),
// 실패=bShouldReposition(우회). range 밖이면 반납. 토큰 lifecycle은 이 서비스+Subsystem 소관.
UCLASS()
class PROJECT_KD_API UBTService_RequestAttackToken : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_RequestAttackToken();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// FindPlayer가 range 기반으로 세팅한 키(읽기 전용) — range 안일 때만 토큰을 요청한다.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanAttackKey;

	// 토큰 보유 여부(쓰기). BT의 Attack 노드 데코레이터가 bCanAttack과 AND로 게이트.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTokenKey;

	// 토큰 대기 중 true → BT가 reposition(우회/포위) 분기. 토큰 획득 시 false.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShouldRepositionKey;
};
