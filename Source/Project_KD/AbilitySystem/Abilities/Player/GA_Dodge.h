// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_ActionBase.h"
#include "GA_Dodge.generated.h"

/**
 * 
 */

// 닷지 방향 (캐릭터 정면 기준). DodgeMontages 배열 인덱스와 매핑.
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward  = 0  UMETA(DisplayName = "Forward"),
	Backward = 1  UMETA(DisplayName = "Backward"),
	Left     = 2  UMETA(DisplayName = "Left"),
	Right    = 3  UMETA(DisplayName = "Right"),
};

// 회피 GA. 입력 방향 따라 4방향 Montage 재생 + Perfect 윈도우 판정 + i-frame GE 적용.
// SB식: 적 공격 윈도우(State.Combat.EnemyAttackHitWindow) 안에서 누르면 Perfect 분기.
UCLASS()
class PROJECT_KD_API UGA_Dodge : public UGA_ActionBase
{
	GENERATED_BODY()

public:
	UGA_Dodge();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// UGA_ActionBase virtual hook — GE 핸들 + 태그 정리.
	virtual void OnCleanup(bool bWasCancelled) override;

	// 4방향 닷지 Montage. 인덱스 = EDodgeDirection 순서 (F/B/L/R).
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TArray<TObjectPtr<UAnimMontage>> DodgeMontages;

	// 퍼펙트 닷지 검사 반경(cm). 이 안의 적 공격 윈도우/발사체를 퍼펙트 대상으로 봄.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Dodge", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float PerfectDodgeCheckRadius = 500.0f;

	// 일반 닷지 무적 GE (짧은 i-frame).
	UPROPERTY(EditDefaultsOnly, Category = "Action|Dodge")
	TSubclassOf<UGameplayEffect> NormalInvincibleGE;

	// Perfect 닷지 무적 GE (긴 i-frame + 카운터 윈도우 태그).
	UPROPERTY(EditDefaultsOnly, Category = "Action|Dodge")
	TSubclassOf<UGameplayEffect> PerfectInvincibleGE;

	// 퍼펙트 닷지 후 카운터 입력 윈도우
	UPROPERTY(EditDefaultsOnly, Category = "Action|Dodge")
	TSubclassOf<UGameplayEffect> CounterWindowGE;

	// Montage 재생 속도.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

private:
	// 입력 방향 → DodgeMontages 인덱스 결정 (캐릭터 정면 기준).
	EDodgeDirection ResolveDodgeDirection() const;

	// 적 ASC에 공격 윈도우 태그 있는지 → Perfect 분기 판정. 적 시스템 없으면 항상 false.
	bool IsInPerfectDodgeWindow(const FGameplayAbilityActorInfo* ActorInfo) const;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();

	// 활성화 중 적용한 무적 GE 핸들 (OnCleanup에서 제거용).
	FActiveGameplayEffectHandle ActiveInvincibleHandle;
};
