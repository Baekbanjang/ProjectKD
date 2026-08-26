// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/KDSprintComponent.h"

void UKDSprintComponent::StartSprint()
{
	// 30fps 간격 속도 보간 타이머 + 풀스프린트 진입 타이머. 중복 호출 방지
	if (bSprintHeld) return;
	bSprintHeld = true;

	UWorld* World = GetWorld();
	if (!World) return;

	if (!World->GetTimerManager().IsTimerActive(SprintTimerHandle))
	{
		World->GetTimerManager().SetTimer(SprintTimerHandle, this,
			&UKDSprintComponent::UpdateSprintSpeed, 0.033f, true);
	}
	World->GetTimerManager().SetTimer(FullSprintTimerHandle, this,
		&UKDSprintComponent::EnterFullSprint, FullSprintTriggerSec, false);
}

void UKDSprintComponent::StopSprint()
{
	bSprintHeld = false;
	ExitFullSprint(); 
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FullSprintTimerHandle);
	}
}

void UKDSprintComponent::ToggleWalk()
{
	bWalkToggled = !bWalkToggled;
	UWorld* World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(SprintTimerHandle))
	{
		World->GetTimerManager().SetTimer(SprintTimerHandle, this,
			&UKDSprintComponent::UpdateSprintSpeed, 0.033f, true);
	}
}

void UKDSprintComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentSpeed = JogSpeed;
}

void UKDSprintComponent::UpdateSprintSpeed()
{
	// 타이머로 30fps 보간. 이동 컴포넌트는 안 건드리고 CurrentSpeed만 굴려 Pawn에 broadcast
	float Target = bWalkToggled ? WalkSpeed : JogSpeed;
	if (bSprintHeld)
	{
		Target = bFullSprintActive ? FullSprintSpeed : SprintSpeed;
	}

	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, Target, 0.033f, SprintInterpSpeed);
	OnMaxWalkSpeedChanged.Broadcast(CurrentSpeed);

	if (FMath::IsNearlyEqual(CurrentSpeed, Target, 1.f))
	{
		CurrentSpeed = Target;
		OnMaxWalkSpeedChanged.Broadcast(CurrentSpeed); // 목표 스냅 보정값도 한 번 더 통지
		if (!bSprintHeld && !bFullSprintActive)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(SprintTimerHandle);
			}
		}
	}
}

void UKDSprintComponent::EnterFullSprint()
{
	bFullSprintActive = true;
	CurrentSpeed = FullSprintSpeed; // 기존처럼 즉시 스냅
	OnMaxWalkSpeedChanged.Broadcast(CurrentSpeed);
}

void UKDSprintComponent::ExitFullSprint()
{
	if (!bFullSprintActive) return;
	bFullSprintActive = false;
}

