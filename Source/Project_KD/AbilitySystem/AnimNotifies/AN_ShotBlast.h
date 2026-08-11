// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ShotBlast.generated.h"

// 콤보 속 총 발사 프레임 노티 — Event.Montage.ShotBlast 전송
UCLASS(meta = (DisplayName = "Shot Blast"))
class PROJECT_KD_API UAN_ShotBlast : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
