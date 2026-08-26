// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KDAnimNotifyState_EnemyAttackWindow.generated.h"

/**
 * 적 공격 몽타주의 "위험 구간"에 배치 — 이 구간 동안 owner ASC에 State.Combat.EnemyAttackHitWindow 태그 부여.
 * 플레이어 GA_Dodge가 회피 입력 순간 이 태그를 쿼리해 퍼펙트 닷지 성립 여부 판정.
 * Begin: AddLooseGameplayTag / End: RemoveLooseGameplayTag
 */
UCLASS()
class PROJECT_KD_API UKDAnimNotifyState_EnemyAttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UKDAnimNotifyState_EnemyAttackWindow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	FGameplayTag AttackWindowTag;
};
