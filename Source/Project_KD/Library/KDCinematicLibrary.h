// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KDCinematicLibrary.generated.h"

class ULevelSequence;
class ALevelSequenceActor;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDCinematicLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 트랜스폼을 직접 기준점으로 시퀀스 재생 (만남점 등 액터 없는 좌표용)
	UFUNCTION(BlueprintCallable, Category = "Cinematic", meta = (WorldContext = "WorldContextObject"))
	static ALevelSequenceActor* PlaySequenceAtTransform(const UObject* WorldContextObject, ULevelSequence* Sequence, const FTransform& Origin, bool bAutoPlay = true);
};
