// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KDGameplayAbility_Skill.h"
#include "KDGameplayAbility_SkillCharge.generated.h"
struct FGameplayEventData;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FKDChargeStep
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Charge")
	FName SectionName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, Category = "Charge", meta = (ClampMin = "0.0"))
	float MinHoldTime = 0.f;
};

UCLASS()
class PROJECT_KD_API UKDGameplayAbility_SkillCharge : public UKDGameplayAbility_Skill
{
	GENERATED_BODY()

protected:
	// 차지
	UPROPERTY(EditDefaultsOnly, Category = "Action|Charge")
	TArray<FKDChargeStep> ChargeSteps;

	// 마무리 섹션
	UPROPERTY(EditDefaultsOnly, Category = "Action|Charge")
	FName EndSectionName = TEXT("End");

	// 릴리즈로 인한 대비 최대 차지 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|Charge", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float MaxHoldTime = 3.f;

	virtual void OnActivated() override;
	virtual void OnCleanup(bool bWasCancelled) override;

private:
	// 진입 섹션 끝에서 몽타주 정지
	void FreezeAtChargeStart();

	// 홀드 시간에 해당하는 섹션의 다임 링크를 EndSection으로 교체 후 재개
	void ReleaseCharge();

	UFUNCTION()
	void OnHoldReleased(FGameplayEventData Payload);

	// 차지 단계 도달 - GC 발신
	void NotifyChargeStep(int32 StepIndex);

	// 단계 타이머 전부 해제
	void ClearStepTimers();

	FTimerHandle FreezeTimerHandle;
	FTimerHandle MaxHoldTimerHandle;
	TArray<FTimerHandle> StepTimerHandles;
	float ChargeStartTime = 0.f;
	bool bChargeReleased = false;
};
