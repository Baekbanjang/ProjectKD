// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_WeaponTraceBase.h"
#include "GameplayTagContainer.h"
#include "GA_EnemyWeaponTraceBase.generated.h"

// 적 근접 공격 GA 베이스. 발동 시 플랜트(braking glide 제거) + 전조 큐.
// 자식은 소켓·몽타주·공격별 거동만 추가.
UCLASS(Abstract)
class PROJECT_KD_API UGA_EnemyWeaponTraceBase : public UGA_WeaponTraceBase
{
	GENERATED_BODY()

public:
	UGA_EnemyWeaponTraceBase();

protected:
	// 이 공격의 전조 큐. 빈 태그면 전조 없음(평범한 공격). BP에서 공격별 지정.
	// 예: GameplayCue.Enemy.Telegraph.Parryable / .Unblockable
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Telegraph", meta = (Categories = "GameplayCue.Enemy.Telegraph"))
	FGameplayTag TelegraphCueTag;

	virtual void OnActivated() override;
	virtual float GetEffectiveMontagePlayRate() const override;
	virtual void OnCleanup(bool bWasCancelled) override;
};
