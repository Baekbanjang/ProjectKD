// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "KDEnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Damage;

// 빙의 시 BT 실행. Pawn의 GetBehaviorTree로 에셋을 받아 구동. BB는 BT 에셋 지정 Blackboard 사용.
// Perception 소유자 — 시야+피격으로 플레이어 인지. 파라미터는 OnPossess에서 EnemyDefinition으로 적용.
// 시야 상실 후 SightMemoryDuration 동안 타겟 기억(추격 유지), 경과 시 잊음.
UCLASS()
class PROJECT_KD_API AKDEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// PathFollowingComponent를 CrowdFollowingComponent로 교체 → Detour Crowd 회피(다수 잡몹 이동 시 안 겹침).
	AKDEnemyAIController(const FObjectInitializer& ObjectInitializer);

	// 현재 인지 중인 타겟(시야 상실 후 기억 유지 포함). 없으면 null — BTService_FindPlayer가 매 틱 읽음.
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	// 거리 기반 교전 facing — 교전 거리 안이면 플레이어를 바라보고(focus + yaw 추적), 밖이면 풀어서
	// 가는 방향을 보게 한다(멀어져 복귀/접근할 때 플레이어 보며 뒷걸음하는 문제 방지). FindPlayer가 매 틱 호출.
	void UpdateCombatFacing(AActor* Target, float DistToTarget);

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// Perception 자극 콜백 — 플레이어만 타겟 파이프에 태움(적끼리 인지 무시).
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

private:
	// 기억 만료 → 타겟 포기.
	void ForgetTarget();

	// 전투=controller yaw로 타겟 추적(윈드업 중에도 몸 돌림) / 비전투(패트롤)=이동방향 회전.
	// 타겟 없이 controller yaw 고정이면 패트롤이 게걸음이 되므로 타겟 유무 전환점에서 토글.
	void SetCombatFacing(bool bCombat);

	// 기억 만료 타이머 (재)시작 — 시야 상실/일회성 피격 자극 공용.
	void StartForgetTimer();

	TWeakObjectPtr<AActor> CurrentTarget;
	FTimerHandle ForgetTargetTimerHandle;

	// OnPossess에서 EnemyDefinition 값으로 캐시.
	float SightMemoryDuration = 4.f;
};
