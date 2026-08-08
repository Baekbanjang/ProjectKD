// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_ActionBase.h"
#include "GA_Shoot.generated.h"

// 사격 GA — 조준 중 좌클릭
// 상체 슬롯 발사 몽타주 1회 재생 발사체는 2단계
UCLASS()
class PROJECT_KD_API UGA_Shoot : public UGA_ActionBase
{
	GENERATED_BODY()

public:
	UGA_Shoot();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> ShootMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;
	
private:
	UFUNCTION()
	void OnMontageFinished();
};
