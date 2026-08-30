// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/KDSlowMotionSubsystem.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UKDSlowMotionSubsystem::RequestSlowMo(float Scale, float Duration, int32 Priority)
{
	// 기능 : 요청을 목록에 넣고 정리
	UWorld* World = GetWorld();
	if (!World || Duration <= 0.f) { return; }
	
	// 끝날 시각 계산
	FKDSlowMoRequest& New = Requests.AddDefaulted_GetRef();
	New.Scale = Scale;
	New.Priority = Priority;
	New.ExpireTime = World->GetRealTimeSeconds() + Duration;
	Recalculate();
}

void UKDSlowMotionSubsystem::Deinitialize()
{
	// 기능 : 레벨 전환 시 기존 시간
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecalcTimer);
		UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
	}
	Super::Deinitialize();
}

void UKDSlowMotionSubsystem::Recalculate()
{
	// 기능 : 완료한 요청 제거 -> 우선순위 높은 시간속도 적용 -> 다음 완료에 타이머 적용
	UWorld* World = GetWorld();
	if (!World) { return; }
	
	const float Now = World->GetRealTimeSeconds(); // 실시간
	
	// 완료한 요청 제거 - ExpireTime 이상 시 제거
	Requests.RemoveAll([Now](const FKDSlowMoRequest& R) { return R.ExpireTime <= Now; });
	
	FTimerManager& Timers = World->GetTimerManager();
	Timers.ClearTimer(RecalcTimer);
	// 비었으면 정상 속도
	if (Requests.Num() == 0)
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
		return;
	}
	
	// 화면을 차지할 요청 고르기 + 다음 알람 시각 찾기
	const FKDSlowMoRequest* Winner = &Requests[0];
	float NextExpire = Requests[0].ExpireTime;
	for (const FKDSlowMoRequest& R : Requests)
	{
		// 우선 순위가 높은 것을 뽑아 해당 타임으로 교체
		if (R.Priority >= Winner->Priority) { Winner = &R; }
		NextExpire = FMath::Min(NextExpire, R.ExpireTime);
	}
	
	UGameplayStatics::SetGlobalTimeDilation(World, Winner->Scale);

	// 목록에서 제일 먼저 끝나는 요청을 다시 호출
	Timers.SetTimer(RecalcTimer, this, &UKDSlowMotionSubsystem::Recalculate,
		FMath::Max(NextExpire - Now, KINDA_SMALL_NUMBER), false);
}
