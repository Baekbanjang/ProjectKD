// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/EQC/EnvQueryContext_AllyEnemies.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Enemy/KDEnemyBaseCharacter.h"
#include "GameFramework/Pawn.h"

void UEnvQueryContext_AllyEnemies::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// querier pawn 식별(Owner가 Controller면 GetPawn) — self 제외 + 위치 기준 반경 게이트에 사용.
	AActor* QuerierActor = Cast<AActor>(QueryInstance.Owner.Get());
	APawn* SelfPawn = Cast<APawn>(QuerierActor);
	if (!SelfPawn)
	{
		if (const AAIController* AICon = Cast<AAIController>(QuerierActor))
			SelfPawn = AICon->GetPawn();
	}
	if (!SelfPawn) return;

	const UWorld* World = SelfPawn->GetWorld();
	if (!World) return;

	const FVector SelfLoc = SelfPawn->GetActorLocation();
	const float RadiusSq = SearchRadius > 0.f ? SearchRadius * SearchRadius : 0.f;

	// 살아있는 다른 적만 모음(나 자신/죽은 적/반경 밖 제외). 적 목록이 따로 없어 월드를 직접 훑음 — 아레나 3~12마리라 가벼움.
	TArray<AActor*> Allies;
	for (TActorIterator<AKDEnemyBaseCharacter> It(World); It; ++It)
	{
		AKDEnemyBaseCharacter* Other = *It;
		if (Other == SelfPawn || Other->IsDead()) continue;
		if (RadiusSq > 0.f && FVector::DistSquared(SelfLoc, Other->GetActorLocation()) > RadiusSq) continue;
		Allies.Add(Other);
	}
	// 줄 게 없으면(빈 목록) 아예 제공 안 함 — 그래야 EQS가 이 기준을 건너뜀.
	if (Allies.Num() == 0) return;

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Allies);
}
