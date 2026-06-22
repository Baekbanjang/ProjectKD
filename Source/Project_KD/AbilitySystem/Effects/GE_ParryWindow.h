// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_ParryWindow.generated.h"

/**
 * 
 */

// Duration GE(기본 0.2s): Target에 State.Combat.Parrying 부여 →
// GE_Damage_Physical이 ApplicationTagRequirements로 reject + Event.Parry.Success 발송.
// GA_Parry가 ActiveGEHandle 저장 후 EndAbility/인터럽트 콜백에서 명시적 제거(태그 leak 방지).
UCLASS()
class PROJECT_KD_API UGE_ParryWindow : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_ParryWindow();
};
