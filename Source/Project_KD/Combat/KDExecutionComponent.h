// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "KDExecutionComponent.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class UKDExecutionProfile;
struct FGameplayEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExecutionBegin);                                  // Pawn/BP: 처형 몽타주 재생 등
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecutionResolved, bool, bSurvived);     // StaggerComponent가 bind

// 처형 판정 사이클. 경직 중 강공 피격 → invuln + 처형 GE + 큐 + 안전망 타이머 → FinishExecution이 결판.
// ASC 작업은 컴포넌트 내부. 모션 재생은 Pawn에 OnExecutionBegin으로 위임(끝나면 FinishExecution 콜백).
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECT_KD_API UKDExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDExecutionComponent();

	UPROPERTY(BlueprintAssignable, Category = "Execution")
	FOnExecutionBegin OnExecutionBegin;

	UPROPERTY(BlueprintAssignable, Category = "Execution")
	FOnExecutionResolved OnExecutionResolved;

	// 적 BP child가 할당. Montage/DamageGE/Cue/Duration/생존여부를 data-driven으로 분기(OCP).
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UKDExecutionProfile> ExecutionProfile;

	// 이번 처형이 데스블로(치명)인지 — HandleExecution 시작 시 결정. 모션/데미지 분기에 사용.
	UFUNCTION(BlueprintPure, Category = "Execution")
	bool IsDeathblow() const { return bDeathblow; }

	// 이번 처형에 재생할 모션 — 데스블로면 DeathblowMontage(폴백 Montage), 생존이면 Montage.
	// Pawn의 OnExecutionBegin이 이걸로 재생, HandleExecution이 안전망 타이머 길이 산정에 사용.
	UFUNCTION(BlueprintPure, Category = "Execution")
	UAnimMontage* GetExecutionMontage() const;

	// 피격 InstigatorTags가 처형 트리거 태그 세트에 해당하는지 — hit-feedback이 처형 히트의 큐를 스킵할 때 사용.
	UFUNCTION(BlueprintPure, Category = "Execution")
	bool IsExecutionTrigger(const FGameplayTagContainer& InstigatorTags) const;

	// BP/몽타주-end safe entry — 처형 데미지 적용 후 OnExecutionResolved 브로드캐스트(생존여부 전달).
	UFUNCTION(BlueprintCallable, Category = "Execution")
	void FinishExecution();

	// Pawn HandleDeath가 호출 — 타이머/invuln/상태 정리(corpse에 cinematic shield 누수 방지).
	void AbortForDeath();

	// 이 사망이 처형 연출 소유인지 — 처형 모션 중이거나 처형 데미지 해소 창(래치, AbortForDeath가 해제).
	// HandleDeath의 죽음 몽타주 스킵 판정 단일 창구(처형 모션이 곧 죽음 연출). 몽타주끝/안전망 어느 경로든 커버.
	bool IsExecutionDeath() const { return bIsBeingExecuted || bResolvingExecution; }

protected:
	// ASC 캐시 + Event.Combat.Hit 구독.
	virtual void BeginPlay() override;

	// Event.Combat.Hit 수신 — IsStaggered(태그) + ExecutionTriggerTags 매치면 HandleExecution.
	void OnHitReceived(const FGameplayEventData* Payload);

	// virtual: 엘리트 다단 override 이음새 (현재 패턴 계승).
	virtual void HandleExecution();

	// 처형을 유발하는 공격 식별 태그. 피격 이벤트 InstigatorTags가 이 중 하나라도 가지면 처형(트리거 식별).
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	FGameplayTagContainer ExecutionTriggerTags;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	bool bIsBeingExecuted = false;

	// 이번 처형이 데스블로(치명)인지 — HandleExecution 시작 시 결정(모션/데미지 분기 공용).
	bool bDeathblow = false;

	// 처형 데미지 해소 창 래치 — FinishExecution이 set, 사망(AbortForDeath)/생존 확정 시 해제 (IsExecutionDeath 참조).
	bool bResolvingExecution = false;

	// 처형 모션 안전망 타이머 (Profile Duration 후 FinishExecution). 몽타주 끝에 일찍 호출 시 clear.
	FTimerHandle ExecutionTimer;

	UPROPERTY()
	TObjectPtr<AActor> ExecutionInstigator = nullptr;
};
