// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/HitStopComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "TimerManager.h"

UHitStopComponent::UHitStopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitStopComponent::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!OwnerASC) { return; }
	
	// 공격 GA가 명중 시 자기 아바타로 보내는 정지 요청 수신
	OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(GameplayTags::Event_Combat_HitStop)
		.AddUObject(this, &UHitStopComponent::OnHitStopEvent);
}

void UHitStopComponent::RequestHitStop(float Duration)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner || Duration <= 0.f) { return; }
	
	FTimerManager& Timers = World->GetTimerManager();
	// 이미 정지 되어있으면 그대로 두고 다시 움직일 시간만 미룸 - 짧은 요청이 긴 정지를 자르는 것 방지
	if (Timers.IsTimerActive(ResumeTimer))
	{
		if (Timers.GetTimerRemaining(ResumeTimer) >= Duration) { return; }
		Timers.ClearTimer(ResumeTimer);
		Timers.SetTimer(ResumeTimer, this, &UHitStopComponent::RestoreTime, Duration, false);
		return;
	}

	// 해당 액터(캐릭터)만 시간 정지(몽타주 클래스 큐칙)
	SavedTimeDilation = Owner->CustomTimeDilation;
	Owner->CustomTimeDilation = 0.f;
	Timers.SetTimer(ResumeTimer, this, &UHitStopComponent::RestoreTime, Duration, false);
}

void UHitStopComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 초기화
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResumeTimer);
	}
	
	RestoreTime();
	Super::EndPlay(EndPlayReason);
}

void UHitStopComponent::OnHitStopEvent(const FGameplayEventData* Payload)
{
	if (Payload)
	{
		RequestHitStop(Payload->EventMagnitude);
	}
}

void UHitStopComponent::RestoreTime()
{
	// 정진 전 속도로 복귀
	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = SavedTimeDilation;
	}
}




