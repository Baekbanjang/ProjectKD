// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "KDGameplayCueNotify_ExecutionCamera.generated.h"

class ALevelSequenceActor;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API AKDGameplayCueNotify_ExecutionCamera : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AKDGameplayCueNotify_ExecutionCamera();

	virtual bool OnActive_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) override;

private:
	// OnRemove에서 정리할 시퀀스 액터
	TWeakObjectPtr<ALevelSequenceActor> SpawnedSeq;
};
