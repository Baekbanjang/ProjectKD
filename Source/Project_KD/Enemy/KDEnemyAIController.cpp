// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/KDEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Enemy/KDEnemyBaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

namespace
{
	// 시야 자극이 현재 살아있는지(성공 + 미만료). 피격 자극의 기억 타이머 arm 여부 판단용.
	bool HasActiveSightStimulus(const UAIPerceptionComponent* Perception, const AActor& Actor)
	{
		if (!Perception) return false;
		const FActorPerceptionInfo* Info = Perception->GetActorInfo(Actor);
		const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
		return Info && Info->LastSensedStimuli.IsValidIndex(SightID)
			&& Info->LastSensedStimuli[SightID].WasSuccessfullySensed()
			&& !Info->LastSensedStimuli[SightID].IsExpired();
	}
}

AKDEnemyAIController::AKDEnemyAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	// Detour Crowd 회피는 씬에 RecastNavMesh + NavMeshBoundsVolume가 있어야 동작(에디터/맵 작업).

	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));

	// 거리/각도 기본값 — 실값은 OnPossess에서 EnemyDefinition으로 덮음(적 1종별 데이터).
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	// 마지막 목격 지점 근처(근접전 빙빙)에선 각도 무시하고 자동 인지 — 등 뒤 점프 시 멍해지는 것 방지.
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.f;
	// 플레이어는 팀 인터페이스 미구현 = Neutral → 전부 켜야 감지됨(아군 오인지는 콜백에서 필터).
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// 피격 자극 — KDEnemyBaseCharacter::OnHitReceived가 ReportDamageEvent로 보고(등 뒤 공격 인지).
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	UAIPerceptionComponent* Perception = GetPerceptionComponent();
	Perception->ConfigureSense(*SightConfig);
	Perception->ConfigureSense(*DamageConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AKDEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(InPawn))
	{
		// 적 1종별 인지 파라미터 적용. LoseSight는 진입 반경보다 약간 넓게(경계 떨림 방지 히스테리시스).
		SightConfig->SightRadius = Enemy->GetSightRadius();
		SightConfig->LoseSightRadius = Enemy->GetSightRadius() * 1.2f;
		SightConfig->PeripheralVisionAngleDegrees = Enemy->GetPeripheralVisionAngle();
		SightMemoryDuration = Enemy->GetSightMemoryDuration();
		if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
		{
			Perception->ConfigureSense(*SightConfig);
			Perception->RequestStimuliListenerUpdate();

			// 다이내믹 바인딩은 CDO(생성자)가 아닌 인스턴스 시점에 — 재빙의에도 안전하게 Unique.
			Perception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AKDEnemyAIController::OnTargetPerceptionUpdated);
		}

		// 시작은 비전투(패트롤) facing — 타겟 인지 시 전투 facing으로 전환.
		SetCombatFacing(false);

		if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
		{
			RunBehaviorTree(BT);
		}
	}
}

void AKDEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 사망 후에도 perception은 계속 자극을 받음(StopLogic은 BT만 정지) — 시체의 타겟/facing 갱신 차단.
	const AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(GetPawn());
	if (Enemy && Enemy->IsDead())
	{
		return;
	}

	// 타겟 파이프는 플레이어 전용 — 적끼리(아군) 자극은 무시.
	if (Actor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Actor;
		SetCombatFacing(true);

		// 시야 자극 → 타이머 해제(상실 이벤트가 따로 옴). 피격은 일회성이라 상실 이벤트 없음 —
		// 시야 미확보일 때만 기억 타이머를 걺(시야 살아있는데 걸면 지워줄 주체 없어 전투 중 포기 버그).
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			GetWorldTimerManager().ClearTimer(ForgetTargetTimerHandle);
		}
		else if (!HasActiveSightStimulus(GetPerceptionComponent(), *Actor))
		{
			StartForgetTimer();
		}
	}
	else if (CurrentTarget.IsValid())
	{
		// 시야 상실 → 기억 유지 후 포기. 이미 도는 타이머는 리셋 안 함 — 상실 반복에 기억 무한 연장 방지.
		// 재목격 시 sensed 분기가 타이머를 지우므로 "마지막 목격 기준 N초" 의미는 유지.
		if (!GetWorldTimerManager().IsTimerActive(ForgetTargetTimerHandle))
		{
			StartForgetTimer();
		}
	}
}

void AKDEnemyAIController::StartForgetTimer()
{
	if (SightMemoryDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(ForgetTargetTimerHandle, this,
			&AKDEnemyAIController::ForgetTarget, SightMemoryDuration, false);
	}
	else
	{
		ForgetTarget();
	}
}

void AKDEnemyAIController::ForgetTarget()
{
	CurrentTarget = nullptr;
	SetCombatFacing(false);
}

void AKDEnemyAIController::SetCombatFacing(bool bCombat)
{
	// controller yaw는 포커스 있을 때만 갱신됨 — 타겟 없이 켜두면 패트롤이 게걸음. 인지/포기 시점에만 토글.
	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (!Char) return;

	// 경직/사망 중엔 facing 동결 — perception 자극이 SetCombatFacing(true)를 되살려 경직 몸이 플레이어를 추적하는 것 방지.
	if (const AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(Char))
	{
		if (Enemy->IsDead() || Enemy->IsStaggered()) return;
	}

	if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = bCombat;      // 전투=컨트롤러 desired로 부드럽게 facing
		Move->bOrientRotationToMovement = !bCombat;         // 비전투=이동방향
	}
}

void AKDEnemyAIController::UpdateCombatFacing(AActor* Target, float DistToTarget)
{
	const AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(GetPawn());
	if (!Enemy) return;

	// 교전거리(EngagementRange) 안이면 응시(공격/포위 스트레이프), 밖이면 풀어서 이동방향 — 포위 링 거리와 일치.
	const float EngageDist = Enemy->GetEngagementRange();
	if (Target && DistToTarget <= EngageDist)
	{
		SetFocus(Target);
		SetCombatFacing(true);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		SetCombatFacing(false);
	}
}
