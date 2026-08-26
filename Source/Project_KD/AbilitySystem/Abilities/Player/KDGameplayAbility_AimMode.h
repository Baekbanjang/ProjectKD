// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_AimMode.generated.h"

class UAnimMontage;
// 총 조준 모드 GA — 홀드 입력
// State.Combat.Aiming 유지 + 진입 몽타주 재생
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_AimMode : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_AimMode();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnCleanup(bool bWasCancelled) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> AimStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> AimEndMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;
};
