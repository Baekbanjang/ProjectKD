// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/KDBaseCharacter.h"
#include "Interface/KDTargetableInterface.h"
#include "KDEnemyBaseCharacter.generated.h"

class UKDKnockbackComponent;
class UWidgetComponent;
class UKDCharacterAttributeSet;
class UKDCombatAttributeSet;
class UGameplayAbility;
class UBehaviorTree;
class UKDHitFeedbackComponent;
class UKDStaggerComponent;
class UKDExecutionComponent;
class UAnimMontage;
class UKDEnemyDefinitionDataAsset;
struct FOnAttributeChangeData;
struct FGameplayEventData;
struct FEnemyAttackEntry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);

// 적 공통 베이스 — ASC = Pawn 직접 소유 (PlayerState 미사용)
// Pawn 담당 = ASC 소유·init | brain | movement | 몽타주 | 넉백 | 사망
// 경직·처형 = 전용 컴포넌트 / 통신 = 델리게이트
UCLASS(Abstract)
class PROJECT_KD_API AKDEnemyBaseCharacter : public AKDBaseCharacter, public IKDTargetableInterface
{
	GENERATED_BODY()

public:
	AKDEnemyBaseCharacter();

	// ASC InitAbilityActorInfo(this, this) + StartupAbilities 부여
	virtual void PossessedBy(AController* NewController) override;

	// 공격 토큰 반납
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 사망 반응 바인딩 진입점 — 상태 전환 | 랙돌 | 디졸브
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const { return bIsDead; }

	// 경직 유무 — StaggerComp 위임
	bool IsStaggered() const;

	// 락온 후보 자격 — 사망 시 제외
	virtual bool CanBeTargeted_Implementation() const override { return !bIsDead; }

	// 락온 조준점
	virtual FVector GetLockOnPoint_Implementation() const override;

	// 락온 유무에 따른 상태 바 표시
	virtual void OnTargeted_Implementation(bool bIsTargeted) override;

	// RunBehaviorTree 용 BT 에셋
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeAsset; }

	// AI 거리 게터 — 값 = EnemyDefinition
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetSightRadius() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetAttackRange() const;

	// 적 정의 DA 읽기 접근
	const UKDEnemyDefinitionDataAsset* GetEnemyDefinition() const { return EnemyDefinition; }

	// 원거리형 유지 거리 (cm) — 0 = 후퇴 X
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetStandoffRange() const;

	// 시야 반각 (도)
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetPeripheralVisionAngle() const;

	// 타겟 기억 유지 시간 (초)
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetSightMemoryDuration() const;

	// 패트롤 반경 (cm) — 0 = 패트롤 X
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetPatrolRadius() const;

	// 패트롤 기준점 = HomeLocation
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	FVector GetHomeLocation() const { return HomeLocation; }

	// 밸런싱 게터 — 값 = EnemyDefinition
	// 공격 몽타주 재생속도 배수 — 기본 1.0
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetAttackSpeedMultiplier() const;

	// 전조 슬로우 배수 — 기본 1.0
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetTelegraphSlowMultiplier() const;

	// 교전 진입 거리 (cm) — 기본 700
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetEngagementRange() const;

	// 회전 속도 (도/초) — 기본 360
	UFUNCTION(BlueprintPure, Category = "Enemy|Balance")
	float GetTurnRate() const;

	// 공격 후보셋 — 값 = EnemyDefinition
	const TArray<FEnemyAttackEntry>& GetAttackEntries() const;

protected:
	// 생성자에서 직접 생성 — Pawn 직접 소유
	UPROPERTY()
	TObjectPtr<UKDCharacterAttributeSet> CharacterAttributes;

	UPROPERTY()
	TObjectPtr<UKDCombatAttributeSet> CombatAttributes;

	// 적 1종 정의 — 스탯 | AI 거리 | 전투 | StartupAbilities | 공격셋
	// 적용 시점 = PossessedBy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UKDEnemyDefinitionDataAsset> EnemyDefinition;

	// 빙의 후 실행할 BT
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// 피격 BoneShake 구동
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDHitFeedbackComponent> HitFeedback;

	// 경직 GAS 상태머신 — Poise 0 -> Stagger GE | 타이머
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDStaggerComponent> StaggerComp;

	// 처형 판정 사이클 — 경직 중 강공 -> invuln | GE | 큐 | 안전망
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDExecutionComponent> ExecutionComp;

	// 피격 밀림 — 거리 지정 + RootMotionSource
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDKnockbackComponent> KnockbackComp;

	// 상태 바 — HP | Poise
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> StateBarWidget;

	// 넉백 중 brain 정지 시간 — 경로추종이 밀림을 되미는 구간
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KnockbackBrainPause = 0.15f;

	// 락온 조준점 소켓
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName LockOnSocketName = TEXT("spine_03");

	// HomeLocation 캐시 + 상태 바 대상 전달
	virtual void BeginPlay() override;

	// Health 0 도달 시 1회 — 콜리전·무브먼트 정리 + OnDeath 브로드캐스트
	virtual void HandleDeath();

	// StaggerComp.OnStaggerBegin 수신 — brain pause + movement freeze
	UFUNCTION()
	void OnStaggerBegin();

	// StaggerComp.OnStaggerRecovered 수신 — brain resume
	UFUNCTION()
	void OnStaggerRecovered();

	// ExecutionComp.OnExecutionBegin 수신 — 처형 몽타주 재생
	UFUNCTION()
	void OnExecutionBegin();

	// KnockbackComp.OnKnockbackBegin 수신 
	UFUNCTION()
	void OnKnockbackBegin();

private:
	// Health 변화 콜백 — 0 이하 시 HandleDeath
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Event.Combat.Hit 수신 — 인지 | 연출 | 수치 | 이동 순 분배
	void OnHitReceived(const FGameplayEventData* Payload);

	// 피격을 AI 인지 자극으로 보고
	void ReportHitToPerception(const FGameplayEventData* Payload);

	// 피격 연출 — 뼈 흔들림 + 타격 큐
	void PlayHitFeedback(const FGameplayEventData* Payload);

	// Poise 차감 — 반환 = 이 차감으로 경직 진입 유무
	bool ApplyPoiseDamage(const FGameplayEventData* Payload);
	
	// 처형 몽타주 종료 콜백 — ExecutionComp FinishExecution 호출
	void OnExecutionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 죽음 몽타주 블렌드아웃 콜백 — 랙돌 인계
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 랙돌 전환 (멱등)
	void EnterRagdoll();

	// 기상 몽타주 종료 콜백 — brain resume
	void OnStaggerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// brain resume 공통 헬퍼
	void ResumeBrainFromStagger();

	// 넉백 구간 종료 brain 재개
	void ResumeBrainFromKnockback();

	// 넉백 brain 재개 타이머
	FTimerHandle KnockbackBrainTimer;

	// 공격 토큰 반납
	void ReturnAttackToken();

	// 사망 유무
	bool bIsDead = false;

	// 패트롤 기준점 — BeginPlay 캐시
	FVector HomeLocation = FVector::ZeroVector;
};
