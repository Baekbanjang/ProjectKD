// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_AllyEnemies.generated.h"

// 근접 적들이 한곳에 뭉치지 않고 플레이어를 둘러싸게 하려는 용도 — 나(질의한 적)를 뺀 살아있는 다른 적들 목록.
// EQS가 "다른 적과 떨어진" 위치를 고르는 기준으로 씀. 월드의 적을 훑음(아레나 3~12마리라 가벼움), 근접 전용.
UCLASS()
class PROJECT_KD_API UEnvQueryContext_AllyEnemies : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	// 이 반경(cm) 안의 적만 셈 — 멀리 떨어진 다른 싸움의 적은 안 끌어옴. 0이면 전부. 너무 많이 훑지 않게 막는 값.
	UPROPERTY(EditAnywhere, Category = "EQS", meta = (ClampMin = "0.0"))
	float SearchRadius = 1500.f;
};
