// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_Parry.generated.h"

/**
 * 
 */

// 가드 GA — 홀드 입력
// 활성화 시점 = 퍼펙트 패링 창 0.15s | 홀드 중 자세 = 가드 로코 PSD | 버튼 해제 = GA 캔슬
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_Parry : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_Parry();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void OnCleanup(bool bWasCancelled) override;

	// 가드 진입 모션
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockStartMontage;

	// 가드 해제 모션 — 재생 X
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockEndMontage;

	// 막힌 히트 플린치 모션
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> BlockHitMontage;

	// 퍼펙트 패링 창 GE — State.Combat.PerfectParryReady 부여
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	TSubclassOf<UGameplayEffect> PerfectParryWindowGE;

	// 홀드 방어 GE — State.Combat.Parrying 부여
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	TSubclassOf<UGameplayEffect> BlockGE;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;


private:
	UFUNCTION()
	void OnBlockStartInterrupted();

	UFUNCTION()
	void OnBlockHitReceived(FGameplayEventData Payload);
	
	FActiveGameplayEffectHandle ActivePerfectWindowHandle;
	FActiveGameplayEffectHandle ActiveBlockHandle;
};
