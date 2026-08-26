// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_PlayerExecution.generated.h"

class UKDPlayerExecutionProfile;
/**
 *
 * 처형 피니셔 GA. 적이 쏜 Event.Combat.ExecutionStarted로 자동발동
 * 적 정면 워프 - 카메라 큐 - 피니셔 몽타주 - OnCleanup에서 복구
 */
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_PlayerExecution : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_PlayerExecution();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void OnCleanup(bool bWasCancelled) override;

	// 플레이어 피니셔 데이터(몽타주/카메라큐/워프거리)
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UKDPlayerExecutionProfile> Profile;

private:
	UFUNCTION()
	void OnFinisherDone();

	// OnRemove 대칭용 — ActivateAbility에서 켠 카메라 큐를 OnCleanup에서 정확히 끄기 위해 캐시
	FGameplayTag ActiveCameraCue;
};
