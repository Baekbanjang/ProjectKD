// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"
#include "GA_PlayerOneShotAttack.generated.h"

class UHitConfirmProfile;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UGA_PlayerOneShotAttack : public UGA_PlayerMeleeAttackBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UHitConfirmProfile> HitConfirmProfile;

	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) override;
	virtual void OnActivated() override;
};
