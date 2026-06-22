// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/EQC/EnvQueryContext_Target.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Pawn.h"

void UEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// querier = 쿼리를 돌린 주체. Run EQS Query 태스크는 보통 AIController를 넘기나, Pawn일 수도 있어 둘 다 처리.
	AActor* QuerierActor = Cast<AActor>(QueryInstance.Owner.Get());
	AAIController* AICon = Cast<AAIController>(QuerierActor);
	if (!AICon)
	{
		if (const APawn* Pawn = Cast<APawn>(QuerierActor))
		{
			AICon = Cast<AAIController>(Pawn->GetController());
		}
	}
	if (!AICon) return;

	const UBlackboardComponent* BB = AICon->GetBlackboardComponent();
	if (!BB) return;

	// FindPlayer가 세팅한 TargetActor를 컨텍스트로 — Donut 제너레이터/Distance 테스트의 중심점.
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));
	if (!Target) return;

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
}
