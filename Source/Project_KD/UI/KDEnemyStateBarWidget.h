// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KDEnemyStateBarWidget.generated.h"

// 적 상태 바 — HP | Poise
// 어트리뷰트 구독과 표시 = BP / C++ = 대상 전달
UCLASS(Abstract)
class PROJECT_KD_API UKDEnemyStateBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 표시 대상 지정
	UFUNCTION(BlueprintImplementableEvent, Category = "StateBar")
	void SetTarget(AActor* NewTarget);
};
