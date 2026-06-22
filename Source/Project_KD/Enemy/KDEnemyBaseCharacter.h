// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "Interface/KDTargetableInterface.h"
#include "KDEnemyBaseCharacter.generated.h"

class UAS_CharacterBase;
class UAS_Combat;
class UGameplayAbility;
class UBehaviorTree;
class UHitFeedbackComponent;
class UStaggerComponent;
class UExecutionComponent;
class UAnimMontage;
class UEnemyDefinitionDataAsset;
struct FOnAttributeChangeData;
struct FGameplayEventData;
struct FEnemyAttackEntry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);

// 적 공통 베이스. ASC는 Pawn-direct(PlayerState 미사용). Pawn은 액추에이션(brain/movement/montage/넉백/사망)과
// ASC 소유·init 담당. 경직/처형은 컴포넌트로 분리, 델리게이트로 통신.
UCLASS(Abstract)
class PROJECT_KD_API AKDEnemyBaseCharacter : public ABaseCharacter, public IKDTargetableInterface
{
	GENERATED_BODY()

public:
	AKDEnemyBaseCharacter();

	// AIController 빙의 시점에 ASC InitAbilityActorInfo(this, this) + StartupAbilities 부여.
	virtual void PossessedBy(AController* NewController) override;

	// 소멸 시 보유 중인 공격 토큰 반납(EncounterSubsystem 슬롯 누수 방지).
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// BT/BP가 사망 반응(상태 전환·래그돌·디졸브)을 바인딩하는 진입점.
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const { return bIsDead; }

	// 경직 중 여부 — AIController가 facing 동결 판단에 사용(경직 몸이 controller yaw로 플레이어를 계속
	// 추적하지 않게). StaggerComp 위임(미존재 시 false).
	bool IsStaggered() const;

	// IKDTargetableInterface — 락온 후보 자격. 사망/처형 중이면 제외. 특수 적은 BP에서 override 가능.
	virtual bool CanBeTargeted_Implementation() const override { return !bIsDead; }

	// 적 락온 위치
	virtual FVector GetLockOnPoint_Implementation() const override;

	// AIController BP가 RunBehaviorTree에 사용하는 진입점. 에셋 지정은 에디터.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeAsset; }

	// AI 거리 게터 — 값은 EnemyDefinition 소유(미할당 시 안전 기본값 반환).
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetSightRadius() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetAttackRange() const;

	// 적 정의 DA 읽기 접근 — GA(예: GA_EnemyHitReact)가 몽타주 등 데이터 조회용.
	const UEnemyDefinitionDataAsset* GetEnemyDefinition() const { return EnemyDefinition; }

	// 활/원거리형이 유지하려는 거리(cm). 0=후퇴 안 함(melee 기본). 이보다 가까우면 kiting 후퇴.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetStandoffRange() const;

	// 시야 반각(도) — AIController가 Perception 시야 콘 설정에 사용.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetPeripheralVisionAngle() const;

	// 시야 상실 후 타겟 기억 유지 시간(초) — AIController의 추격 포기 타이머.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetSightMemoryDuration() const;

	// 패트롤 반경(cm). 0=패트롤 안 함. BTTask_FindPatrolPoint가 읽음.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetPatrolRadius() const;

	// 패트롤 기준점 — BeginPlay 시점의 스폰 위치.
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	FVector GetHomeLocation() const { return HomeLocation; }

	// --- 밸런싱 게터 (EnemyDefinition 소유, 미할당 시 안전 기본값) ---
	// 공격 몽타주 재생속도 배수(기본 1.0). GA_EnemyWeaponTraceBase가 GetEffectiveMontagePlayRate에서 곱함.
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetAttackSpeedMultiplier() const;

	// 전조 엇박 슬로우 배수(기본 1.0). ANS_WindupSlow가 SlowRate에 곱함.
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetTelegraphSlowMultiplier() const;

	// 교전 진입 거리(cm, 기본 700). BTService_RequestAttackToken이 읽음.
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetEngagementRange() const;

	// 회전 속도(도/초, 기본 360). PossessedBy에서 CMC RotationRate.Yaw에 적용.
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetTurnRate() const;

	// 공격 후보셋(EnemyDefinition 소유) — UBTTask_SelectAttack이 읽음. null이면 빈 배열.
	const TArray<FEnemyAttackEntry>& GetAttackEntries() const;

protected:
	// Pawn-direct ASC가 보유. PlayerState 경로 대비 enemy는 생성자에서 직접 생성.
	UPROPERTY()
	TObjectPtr<UAS_CharacterBase> CharacterAttributes;

	UPROPERTY()
	TObjectPtr<UAS_Combat> CombatAttributes;

	// 적 1종 정의(스탯/AI거리/전투/StartupAbilities/공격셋). BP child는 외형만, 수치·행동은 이 에셋이 소유.
	// PossessedBy에서 적용. null이면 AS 생성자 기본값 사용(게터도 안전 기본값 반환).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UEnemyDefinitionDataAsset> EnemyDefinition;

	// AIController가 빙의 후 실행할 BT. 에셋은 에디터 지정.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// 피격 시 BoneShake 구동 (GCN_HitImpact가 TriggerBoneShake 호출). Player와 동일 패턴.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHitFeedbackComponent> HitFeedback;

	// 경직 GAS 상태머신(Poise 0 → stagger GE/타이머). 액추에이션은 델리게이트로 위임받음.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UStaggerComponent> StaggerComp;

	// 처형 판정 사이클(경직 중 강공 → invuln/GE/큐/안전망). 모션 재생은 델리게이트로 위임.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UExecutionComponent> ExecutionComp;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName LockOnSocketName = TEXT("spine_03");

	// 패트롤 홈 위치 캐시(스폰 지점).
	virtual void BeginPlay() override;

	// Health 0 도달 시 1회 호출. 콜리전/무브먼트 정리 + OnDeath 브로드캐스트.
	virtual void HandleDeath();

	// StaggerComponent.OnStaggerBegin → brain pause + movement freeze (스턴 몽타주가 안 미끄러지게).
	UFUNCTION()
	void OnStaggerBegin();

	// StaggerComponent.OnStaggerRecovered → brain resume (사망이 아닐 때만).
	UFUNCTION()
	void OnStaggerRecovered();

	// ExecutionComponent.OnExecutionBegin → 처형 몽타주 재생(끝나면 ExecutionComp->FinishExecution).
	UFUNCTION()
	void OnExecutionBegin();

private:
	// ASC Health 변화 콜백 — 0 이하면 HandleDeath. AS는 데이터만 보유, 사망 판정은 캐릭터 책임.
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Event.Combat.Hit 수신(공유 OnWeaponHit이 전송) — poise 차감 + 넉백. 처형 트리거는 ExecutionComponent가
	// 별도 구독 처리(ASC 다중 구독자 순서 미보장). 경직 중이면 early-return(처형 케이스 분리).
	void OnHitReceived(const FGameplayEventData* Payload);

	// 처형 몽타주 종료 콜백 → ExecutionComp->FinishExecution(치명 시 랙돌 안착). 안전망 타이머 중복은 가드.
	void OnExecutionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 죽음 몽타주 블렌드아웃 시작 콜백 → 죽음 포즈 그대로 랙돌 인계(지형 안착, 하이브리드).
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 랙돌 전환(멱등) — 처형사망 즉시/죽음 몽타주 블렌드아웃/백스톱 타이머 공용 종착점.
	void EnterRagdoll();

	// 경직 회복 시 기상(End 섹션) 몽타주 종료 콜백 → brain resume. bInterrupted면 재경직/사망이 새 몽타주로
	// 점유한 것이므로 no-op(그 권위가 brain을 관리).
	void OnStaggerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// BT brain resume 공통 헬퍼 — 경직 회복의 즉시 복귀/기상 후 복귀 두 경로가 공유.
	void ResumeBrainFromStagger();

	// 공격 토큰 반납(경직/사망 시) — 슬롯 비워 대기 적이 받게. Subsystem 없으면 no-op.
	void ReturnAttackToken();

	bool bIsDead = false;

	// 패트롤 기준점 — BeginPlay에서 캐시.
	FVector HomeLocation = FVector::ZeroVector;
};
