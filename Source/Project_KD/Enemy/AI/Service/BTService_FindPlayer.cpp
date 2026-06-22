// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Service/BTService_FindPlayer.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/KDEnemyAIController.h"
#include "Enemy/KDEnemyBaseCharacter.h"

UBTService_FindPlayer::UBTService_FindPlayer()
{
	NodeName = TEXT("Find Player");
	Interval = 0.3f;
	RandomDeviation = 0.05f;

	// Restrict the editor key pickers to their expected types.
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindPlayer, TargetActorKey), AActor::StaticClass());
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FindPlayer, CanAttackKey));
}

void UBTService_FindPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
		CanAttackKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_FindPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AKDEnemyAIController* AICon = Cast<AKDEnemyAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB)
	{
		return;
	}

	// 인지(시야 콘/피격 + 상실 후 기억)는 controller perception이 판정 — 이 서비스는 BB로 미러만.
	APawn* SelfPawn = AICon->GetPawn();
	AActor* Target = AICon->GetCurrentTarget();
	if (!SelfPawn || !Target)
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, false);
		AICon->ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	// Per-enemy radii live on the pawn so one BT/service serves every melee type (BP-tuned).
	float AttackRange = 150.f;
	if (const AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(SelfPawn))
	{
		AttackRange = Enemy->GetAttackRange();
	}

	const float Dist = FVector::Dist(SelfPawn->GetActorLocation(), Target->GetActorLocation());

	// 사거리 안이어도 시야가 막혀 있으면 공격 금지(벽 너머 타격 방지). 추적(TargetActor)은 유지.
	bool bHasLOS = true;
	if (Dist <= AttackRange)
	{
		if (UWorld* World = SelfPawn->GetWorld())
		{
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(FindPlayerLOS), false, SelfPawn);
			Params.AddIgnoredActor(Target);
			bHasLOS = !World->LineTraceSingleByChannel(
				Hit, SelfPawn->GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, Params);
		}
	}

	BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Target);
	BB->SetValueAsBool(CanAttackKey.SelectedKeyName, Dist <= AttackRange && bHasLOS);

	// 교전 거리 안이면 플레이어를 바라보고(공격 윈드업 중에도 yaw 추적), 멀면 풀어서 이동 방향을 보게.
	AICon->UpdateCombatFacing(Target, Dist);
}
