// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_PlayerTurn.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_PlayerTurn : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_PlayerTurn();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// UGA_ActionBase virtual hook — orient-to-movement 복원 + State.Movement.Turning 태그 제거.
	virtual void OnCleanup(bool bWasCancelled) override;

	// 180도 턴 몽타주. [0] = 우(R, +각도), [1] = 좌(L, -각도).
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TArray<TObjectPtr<UAnimMontage>> TurnMontages;

	// Montage 재생 속도.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();

	// 활성화 시 orient-to-movement를 껐는지 기록 → OnCleanup에서 원래 값으로 복원.
	bool bRestoreOrientRotation = false;
};
