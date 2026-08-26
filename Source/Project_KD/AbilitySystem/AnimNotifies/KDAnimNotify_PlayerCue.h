// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KDAnimNotify_PlayerCue.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDAnimNotify_PlayerCue : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// 이 프레임에 owner ASC로 쏠 큐 (카운터 몽타주 = GameplayCue.Combat.CounterThrust)
	UPROPERTY(EditAnywhere, Category = "Cue")
	FGameplayTag CueTag;
};
