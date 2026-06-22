// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_Target.generated.h"

// EQS 기준점 = 적의 현재 타겟(플레이어). querier가 아니라 BB의 TargetActor를 읽어 제공.
// 활·melee 공용(둘 다 "플레이어 기준 어디"를 물음).
UCLASS()
class PROJECT_KD_API UEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	// 읽을 타겟 키명. FindPlayer의 TargetActor 키명과 일치시킬 것(EQS는 KeySelector 불가 → FName 직접).
	UPROPERTY(EditAnywhere, Category = "EQS")
	FName TargetKeyName = TEXT("TargetActor");
};
