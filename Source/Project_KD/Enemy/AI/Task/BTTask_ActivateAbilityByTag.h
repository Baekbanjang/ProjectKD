// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateAbilityByTag.generated.h"

// Activates a pawn ASC ability by tag and stays InProgress until that ability ends,
// so the BT does not advance mid-montage.
UCLASS()
class PROJECT_KD_API UBTTask_ActivateAbilityByTag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateAbilityByTag();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 고정 어빌리티 태그 — BT에서 지정. AbilityTagNameKey 설정 시 그쪽(데이터드리븐)이 우선.
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag ActivationTag;

	// 선택: 이 BB Name 키 값(SelectAttack이 씀)을 ActivationTag 대신 사용. 미설정이면 ActivationTag만.
	UPROPERTY(EditAnywhere, Category = "Ability")
	FBlackboardKeySelector AbilityTagNameKey;
};
