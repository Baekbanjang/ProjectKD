// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/SprintComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/AS_Player.h"


void USprintComponent::StartSprint()
{
	// 30fps 간격 속도 보간 타이머 + 풀스프린트 진입 타이머. 중복 호출 방지
	if (bSprintHeld) return;
	bSprintHeld = true;

	UWorld* World = GetWorld();
	if (!World) return;

	if (!World->GetTimerManager().IsTimerActive(SprintTimerHandle))
	{
		World->GetTimerManager().SetTimer(SprintTimerHandle, this,
			&USprintComponent::UpdateSprintSpeed, 0.033f, true);
	}
	World->GetTimerManager().SetTimer(FullSprintTimerHandle, this,
		&USprintComponent::EnterFullSprint, FullSprintTriggerSec, false);
}

void USprintComponent::StopSprint()
{
	bSprintHeld = false;
	ExitFullSprint(); 
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FullSprintTimerHandle);
	}
}

void USprintComponent::ToggleWalk()
{
	bWalkToggled = !bWalkToggled;
	UWorld* World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(SprintTimerHandle))
	{
		World->GetTimerManager().SetTimer(SprintTimerHandle, this,
			&USprintComponent::UpdateSprintSpeed, 0.033f, true);
	}
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentSpeed = JogSpeed;
}

void USprintComponent::UpdateSprintSpeed()
{
	// 현재 풀스프린트 중이라면 스태미나 소진 - 0 이하되면 해제
	if (bFullSprintActive)
	{
		if (UAbilitySystemComponent* ASC = ResolveASC())
		{
			if (ASC->GetNumericAttribute(UAS_Player::GetStaminaAttribute()) <= 0.f)
			{
				ExitFullSprint();
			}
		}
	}
	
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

void USprintComponent::EnterFullSprint()
{
	bFullSprintActive = true;
	CurrentSpeed = FullSprintSpeed; // 기존처럼 즉시 스냅
	OnMaxWalkSpeedChanged.Broadcast(CurrentSpeed);

	// 풀스프린트 진입 -> 스태미나 지속 소모 GE 적용
	if (FullSprintStaminaCostGE)
	{
		if (UAbilitySystemComponent* ASC = ResolveASC())
		{
			const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(FullSprintStaminaCostGE, 1.0f, Ctx);
			if (Spec.IsValid())
			{
				SprintStaminaCostHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}
}

void USprintComponent::ExitFullSprint()
{
	if (!bFullSprintActive) return;
	bFullSprintActive = false;

	// 지속소모 GE 제거 -> 회복차단 태그도 사라져 회복 재개. 속도는 보간이 SprintSpeed로 복귀
	if (SprintStaminaCostHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ResolveASC())
		{
			ASC->RemoveActiveGameplayEffect(SprintStaminaCostHandle);
		}
		SprintStaminaCostHandle = FActiveGameplayEffectHandle();
	}
}

UAbilitySystemComponent* USprintComponent::ResolveASC() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}
