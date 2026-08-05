// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitStopComponent.generated.h"

struct FGameplayEventData;

// 타격 순간 액터 정지 - Event.Combat.HitStop 수신 - 대상은 소유 액터 자신
// 정지 시간은 GA가 정해서 보냄, 겹치면 더 긴 쪽으로

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UHitStopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitStopComponent();

	UFUNCTION(BlueprintCallable, Category = "HitStop")
	void RequestHitStop(float Duration);

	UFUNCTION(BlueprintPure, Category = "HitStop")
	bool IsHitStopActive() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:	
	void OnHitStopEvent(const FGameplayEventData* Payload);
	void RestoreTime();
	
	// 정지 전 속도
	float SavedTimeDilation = 1.f;
	FTimerHandle ResumeTimer;
};
