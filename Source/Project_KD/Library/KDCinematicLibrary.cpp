// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/KDCinematicLibrary.h"

#include "DefaultLevelSequenceInstanceData.h"
#include "LevelSequenceActor.h"
#include "Engine/Engine.h"

ALevelSequenceActor* UKDCinematicLibrary::PlaySequenceAtTransform(const UObject* WorldContextObject,
	ULevelSequence* Sequence, const FTransform& Origin, bool bAutoPlay)
{
	if (!Sequence) { return nullptr; }

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World) { return nullptr; }

	// 디퍼드 스폰 - BeginPlay 전에 시퀀스/Origin 다 세팅해야 자동재생이 제대로 걸림
	ALevelSequenceActor* SeqActor = World->SpawnActorDeferred<ALevelSequenceActor>(ALevelSequenceActor::StaticClass(), FTransform::Identity);
	if (!SeqActor) { return nullptr; }

	SeqActor->PlaybackSettings.bAutoPlay = bAutoPlay;
	SeqActor->SetSequence(Sequence);

	// Transform Origin = Origin 액터 트랜스폼(스폰 순간 스냅샷) - 시퀀스 전체가 이 기준으로 재계산
	SeqActor->bOverrideInstanceData = true;
	UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(SeqActor->DefaultInstanceData);
	if (!InstanceData)
	{
		InstanceData = NewObject<UDefaultLevelSequenceInstanceData>(SeqActor);
		SeqActor->DefaultInstanceData = InstanceData;
	}
	InstanceData->TransformOrigin = Origin;

	SeqActor->FinishSpawning(FTransform::Identity);
	return SeqActor;
}

ALevelSequenceActor* UKDCinematicLibrary::PlaySequenceAtActor(const UObject* WorldContextObject, ULevelSequence* Sequence,
	AActor* OriginActor, bool bAutoPlay)
{
	if (!OriginActor) { return nullptr; }
	return PlaySequenceAtTransform(WorldContextObject, Sequence, OriginActor->GetActorTransform(), bAutoPlay);
}
