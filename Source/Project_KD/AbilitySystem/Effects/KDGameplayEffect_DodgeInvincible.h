// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "KDGameplayEffect_DodgeInvincible.generated.h"

/**
 * 
 */

// Duration GE(기본 0.5s):
// Target에 State.Combat.Invulnerable 부여 → GE_Damage_Physical이 ApplicationTagRequirements로 자동 reject.
// 몽타주 인터럽트 시 태그 leak 방지 위해 GA_Dodge가 ActiveGEHandle 저장 후 콜백에서
// 명시적 RemoveActiveGameplayEffect 호출(이중 안전망).
UCLASS()
class PROJECT_KD_API UKDGameplayEffect_DodgeInvincible : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UKDGameplayEffect_DodgeInvincible();
};
