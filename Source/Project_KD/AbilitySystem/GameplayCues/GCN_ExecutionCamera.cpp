// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayCues/GCN_ExecutionCamera.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "AbilitySystem/Abilities/Player/Execution/KDPlayerExecutionProfile.h"
#include "GameFramework/PlayerController.h"   
#include "Engine/World.h"                     
#include "Library/KDCinematicLibrary.h"

AGCN_ExecutionCamera::AGCN_ExecutionCamera()
{
	bAutoDestroyOnRemove = true; // 큐 제거 시 GC 액터 정리
}

bool AGCN_ExecutionCamera::OnActive_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
{
	// GA가 컨텍스트에 실어보낸 Profile에서 시퀀스 꺼냄. 시퀀스 비면 시네마틱 스킵
	const UKDPlayerExecutionProfile* Profile = Cast<UKDPlayerExecutionProfile>(Parameters.EffectContext.GetSourceObject());
	if (!Target || !Profile || !Profile->FinisherSequence) { return false; }

	// 기준점 = 큐에 실린 만남점 좌표 + 적을 향한 플레이어 회전. 없으면 Target 트랜스폼 폴백
	FTransform OriginXform = Target->GetActorTransform();
	if (Parameters.EffectContext.IsValid() && Parameters.EffectContext.Get()->HasOrigin())
	{
		OriginXform = FTransform(Target->GetActorRotation(), Parameters.EffectContext.Get()->GetOrigin());
	}
	SpawnedSeq = UKDCinematicLibrary::PlaySequenceAtTransform(Target, Profile->FinisherSequence, OriginXform, true);
	return true;
}

bool AGCN_ExecutionCamera::OnRemove_Implementation(AActor* Target, const FGameplayCueParameters& Parameters)
{
	if (ALevelSequenceActor* SeqActor = SpawnedSeq.Get())
	{
		if (ULevelSequencePlayer* Player = SeqActor->GetSequencePlayer()) { Player->Stop(); }
		SeqActor->Destroy();
	}
	return true;
}
