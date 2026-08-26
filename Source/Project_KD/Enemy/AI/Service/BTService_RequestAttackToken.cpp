// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/Service/BTService_RequestAttackToken.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/AI/KDEncounterSubsystem.h"
#include "Enemy/KDEnemyBaseCharacter.h"

UBTService_RequestAttackToken::UBTService_RequestAttackToken()
{
	NodeName = TEXT("Request Attack Token");
	Interval = 0.2f;
	RandomDeviation = 0.03f;

	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, CanAttackKey));
	HasTokenKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, HasTokenKey));
	ShouldRepositionKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RequestAttackToken, ShouldRepositionKey));
}

void UBTService_RequestAttackToken::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		CanAttackKey.ResolveSelectedKey(*BBAsset);
		HasTokenKey.ResolveSelectedKey(*BBAsset);
		ShouldRepositionKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_RequestAttackToken::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return;

	AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(AICon->GetPawn());
	UWorld* World = AICon->GetWorld();
	UKDEncounterSubsystem* Encounter = World ? World->GetSubsystem<UKDEncounterSubsystem>() : nullptr;
	if (!Enemy || !Encounter) return;

	// 원거리 kiter(StandoffRange>0)는 토큰 면제 — 토큰은 근접 동시공격 제한 자원이라 원거리엔 부적합
	// (활 다수가 동시 사격 가능). 항상 attack 분기 허용 → 사거리 안이면 멈춰 사격, reposition 안 탐.
	// 거리 관리는 kiting 노드가 담당. melee/단검투척(StandoffRange=0)은 아래 토큰 경쟁 유지.
	if (Enemy->GetStandoffRange() > 0.f)
	{
		BB->SetValueAsBool(HasTokenKey.SelectedKeyName, true);
		BB->SetValueAsBool(ShouldRepositionKey.SelectedKeyName, false);
		return;
	}

	// 교전거리(EngagementRange) 기준 — 공격거리(~150)보다 넓게 잡아 멀리서부터 포위 형성.
	// 공격거리 기준으로 쓰면 플레이어가 적 코앞에 들어가야 포위가 시작되는 버그 있었음.
	const float EngagementRange = Enemy->GetEngagementRange();
	float DistToPlayer = TNumericLimits<float>::Max();
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		DistToPlayer = FVector::Dist(Enemy->GetActorLocation(), Player->GetActorLocation());
	}

	// 교전거리 밖 — 토큰 반납, 추격만(BT Chase). 멀리서 슬롯 점유 방지.
	if (DistToPlayer > EngagementRange)
	{
		Encounter->ReturnToken(Enemy);
		BB->SetValueAsBool(HasTokenKey.SelectedKeyName, false);
		BB->SetValueAsBool(ShouldRepositionKey.SelectedKeyName, false);
		return;
	}

	// 교전거리 안 — 토큰 요청. 보유 시 공격 진입(Chase→Attack), 미보유 시 포위(reposition).
	const bool bHasToken = Encounter->RequestToken(Enemy);
	BB->SetValueAsBool(HasTokenKey.SelectedKeyName, bHasToken);
	BB->SetValueAsBool(ShouldRepositionKey.SelectedKeyName, !bHasToken);
}
