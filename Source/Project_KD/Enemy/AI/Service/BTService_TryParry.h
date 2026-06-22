// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTService_TryParry.generated.h"

// 플레이어 공격에 반응하는 막기. 플레이어가 공격 중(State.Combat.Attacking)이고 ParryRange 안이면 매 틱
// ParryChance 확률로 적의 막기 GA를 바로 발동(하던 행동을 끊고). 막기 GA가 없는 적은 그냥 발동 실패라 무해.
UCLASS()
class PROJECT_KD_API UBTService_TryParry : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_TryParry();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 플레이어(공격 상태/거리 판정 대상). FindPlayer가 세팅한 TargetActor 키와 일치시킬 것.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 발동할 패링 GA 식별 태그(예: Ability.Enemy.Grunt.Parry).
	UPROPERTY(EditAnywhere, Category = "Parry")
	FGameplayTag ParryAbilityTag;

	// 이 거리 안에서만 패링 시도(근접 가드).
	UPROPERTY(EditAnywhere, Category = "Parry", meta = (ClampMin = "0.0"))
	float ParryRange = 250.0f;

	// 조건 충족 시 틱당 발동 확률(0~1). 서비스 Interval과 함께 빈도를 결정.
	UPROPERTY(EditAnywhere, Category = "Parry", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryChance = 0.5f;
};
