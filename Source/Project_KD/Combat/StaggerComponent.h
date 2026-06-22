// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"   // FActiveGameplayEffectHandle (value member)
#include "StaggerComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaggerBegin);      // Pawn이 bind: brain pause + movement freeze
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaggerRecovered);  // Pawn이 bind: brain resume

// F10 경직 GAS 상태머신. Poise 0 도달 → Stagger GE 부여(State.Combat.Staggered) → 타이머/처형생존으로 복귀.
// ASC 작업(GE 적용/제거, 태그 읽기)은 컴포넌트 내부. Pawn 액추에이션(brain/movement)은 델리게이트 위임.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECT_KD_API UStaggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaggerComponent();

	UPROPERTY(BlueprintAssignable, Category = "Stagger")
	FOnStaggerBegin OnStaggerBegin;

	UPROPERTY(BlueprintAssignable, Category = "Stagger")
	FOnStaggerRecovered OnStaggerRecovered;

	// ASC State.Combat.Staggered 태그 읽기 — 처형 트리거가 직접 포인터 대신 이걸로 경직 확인.
	UFUNCTION(BlueprintPure, Category = "Stagger")
	bool IsStaggered() const;

	// Stagger GE 제거 + Poise 풀 리셋 + OnStaggerRecovered 브로드캐스트(재경직 가능).
	UFUNCTION(BlueprintCallable, Category = "Stagger")
	void RecoverFromStagger();

	// ExecutionComponent.OnExecutionResolved에 bind — 생존 시 RecoverFromStagger(즉사 시 Pawn 사망 정리).
	UFUNCTION()
	void HandleExecutionResolved(bool bSurvived);

	// Pawn HandleDeath가 호출 — 타이머/GE핸들 정리(corpse에 stagger 잔류 방지).
	void AbortForDeath();

protected:
	// ASC 캐시 + Poise-changed 구독.
	virtual void BeginPlay() override;

	// ASC Poise 변화 콜백 — 0 도달 + 미경직이면 BeginStagger.
	void OnPoiseChanged(const FOnAttributeChangeData& Data);

	void BeginStagger();

	// 자동 복귀 타이머 콜백 — 처형 진행 중(State.Combat.Invulnerable)이면 skip.
	void OnStaggerTimeout();

	// F10 Stagger GE (BP 지정 가능, 기본값 = UGE_Stagger). Infinite + GrantedTag(State.Combat.Staggered).
	UPROPERTY(EditDefaultsOnly, Category = "Stagger")
	TSubclassOf<UGameplayEffect> StaggerEffectClass;

	// 경직 자동 복귀 시간(초). 이 안에 처형이 없으면 자동 회복.
	UPROPERTY(EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float StaggerDuration = 3.0f;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	// Stagger GE 핸들. BeginStagger 적용 / RecoverFromStagger 제거.
	FActiveGameplayEffectHandle StaggerEffectHandle;

	// 경직 자동 복귀 타이머 (StaggerDuration 후 RecoverFromStagger). 처형이 일찍 호출 시 clear.
	FTimerHandle StaggerTimeoutTimer;
};
