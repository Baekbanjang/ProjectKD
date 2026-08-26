// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KDSprintComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxWalkSpeedChanged, float, NewSpeed);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UKDSprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintAssignable, Category = "Sprint")
	FOnMaxWalkSpeedChanged OnMaxWalkSpeedChanged;

	void StartSprint();
	void StopSprint();
	void ToggleWalk();

	bool IsSprinting() const { return bSprintHeld; }
	bool IsFullSprinting() const { return bFullSprintActive; }
	bool IsWalking() const { return bWalkToggled; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="100",ClampMax="1500"))
	float WalkSpeed = 250.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="100",ClampMax="1500"))
	float JogSpeed = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="100",ClampMax="2000"))
	float SprintSpeed = 700.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="100",ClampMax="2500"))
	float FullSprintSpeed = 800.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="1.0",ClampMax="10.0"))
	float FullSprintTriggerSec = 4.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Sprint", meta=(ClampMin="0.5",ClampMax="10.0"))
	float SprintInterpSpeed = 6.0f;

private:
	UFUNCTION() void UpdateSprintSpeed(); // 타이머 콜백: CurrentSpeed 보간 → broadcast
	UFUNCTION() void EnterFullSprint();

	float CurrentSpeed = 500.f;   // 진실원. BeginPlay에서 JogSpeed로 시드(첫 스프린트 끊김 방지)
	bool  bSprintHeld = false;
	bool  bFullSprintActive = false;
	bool  bWalkToggled = false;
	FTimerHandle SprintTimerHandle;
	FTimerHandle FullSprintTimerHandle;

	UFUNCTION() void ExitFullSprint();
};
