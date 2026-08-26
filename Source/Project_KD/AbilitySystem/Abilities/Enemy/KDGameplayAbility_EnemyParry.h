// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_EnemyParry.generated.h"

class UAnimMontage;
class UGameplayEffect;

// 적 방어형 패링 GA. 발동 시 State.Combat.Parrying 부여 + 패링 몽타주 재생.
// 가드 윈도우 = 몽타주 길이. AS_Combat이 Parrying 태그를 보고 데미지를 막음(플레이어 패링과 공유 경로).
// PerfectParryReady(슬로모) 경로 미사용 — 플레이어 무경직, 적은 데미지만 막는 비대칭 모델.
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_EnemyParry : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_EnemyParry();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void OnCleanup(bool bWasCancelled) override;

	// Guard 섹션(가드 스탠스) 재생 → 패링 성공 시만 ParrySuccessSection으로 점프.
	// Guard 섹션 Next=None으로 설정할 것(루프 방지). Success는 점프로만 도달.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	FName ParrySuccessSection = TEXT("Success");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	// State.Combat.Parrying 부여 GE(Infinite). GA 종료 시 OnCleanup에서 제거.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	TSubclassOf<UGameplayEffect> ParryGE;

	// 패링 성공 시 클래시 GC. 기본값 = GameplayCue.Combat.ParryClash(생성자 지정), BP child가 덮어쓰기 가능.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Parry")
	FGameplayTag ParryClashCueTag;

private:
	UFUNCTION()
	void OnParryMontageEnded();

	UFUNCTION()
	void OnParrySuccess(FGameplayEventData Payload);

	FActiveGameplayEffectHandle ActiveParryHandle;
};
