// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "KDGameplayEffect_Stagger.generated.h"

// 경직 상태 GE (F10). Infinite — AKDEnemyBaseCharacter가 StaggerDuration 타이머로 제거.
// Target에 State.Combat.Staggered(GA 차단) 부여,
// GameplayCue.Combat.Staggered 발동(스턴 VFX). 튜닝값 없음 — 순수 구조라 BP child 불필요.
UCLASS()
class PROJECT_KD_API UKDGameplayEffect_Stagger : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UKDGameplayEffect_Stagger();
};
