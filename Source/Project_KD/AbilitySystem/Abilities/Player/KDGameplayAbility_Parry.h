// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_Parry.generated.h"

/**
 * 
 */

// 가드 GA. Hold 입력 패턴: 시작 시점에 Perfect Parry 윈도우(0.15s) 부여,
// 그 사이 적 공격 들어오면 Perfect 분기. 떼는 순간 캔슬 → Block End 모션 후 종료
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_Parry : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_Parry();

protected:
	void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void OnCleanup(bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockEndMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockHitMontage;

	// Perfect Parry 윈도우 GE
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	TSubclassOf<UGameplayEffect> PerfectParryWindowGE;

	// 홀드 방어 GE(Infinite) — State.Combat.Parrying 부여(50% 감소)
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	TSubclassOf<UGameplayEffect> BlockGE;

	// 퍼펙트 윈도우 길이(초). 
	UPROPERTY(EditAnywhere, Category = "Action|Parry", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float PerfectParryWindowSec = 0.2f;
	
	// Montage 재생 속도.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;


private:
	UFUNCTION()
	void OnBlockStartCompleted();

	UFUNCTION()
	void OnBlockStartInterrupted();

	UFUNCTION()
	void OnBlockHitReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnBlockHitMontageEnded();

	void PlayBlockLoop();

	FActiveGameplayEffectHandle ActivePerfectWindowHandle;
	FActiveGameplayEffectHandle ActiveBlockHandle;
};
