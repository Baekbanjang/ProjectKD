// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "KDGameplayEffect_AmmoCost.generated.h"

// 사격 탄약 소모 — Instant, Ammo -1

UCLASS()
class PROJECT_KD_API UKDGameplayEffect_AmmoCost : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UKDGameplayEffect_AmmoCost();
};
