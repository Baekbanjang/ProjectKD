// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/AI/KDEncounterSubsystem.h"

#include "Engine/World.h"
#include "Enemy/KDEnemyBaseCharacter.h"
#include "HAL/IConsoleManager.h"

static int32 GKDMaxAttackers = 2;
static FAutoConsoleVariableRef CVarKDMaxAttackers(
    TEXT("kd.Encounter.MaxAttackers"), GKDMaxAttackers,
    TEXT("동시 공격 토큰 상한 (기본 2)."), ECVF_Default);

static float GKDTokenHoldTime = 2.5f;
static FAutoConsoleVariableRef CVarKDTokenHoldTime(
    TEXT("kd.Encounter.TokenHoldTime"), GKDTokenHoldTime,
    TEXT("토큰 강제 회수 시각 초 (기본 2.5)."), ECVF_Default);

bool UKDEncounterSubsystem::RequestToken(AKDEnemyBaseCharacter* Enemy)
{
	if (!Enemy) return false;

	// 죽은 holder(weak null) 청소 — 사망 적이 슬롯을 잡고 있지 않게.
	TokenHolders.RemoveAll([](const FTokenSlot& Slot) { return !Slot.Holder.IsValid(); });

	// 이미 보유 중이면 유지.
	for (const FTokenSlot& Slot : TokenHolders)
	{
		if (Slot.Holder.Get() == Enemy) return true;
	}

	const float Now = GetWorld()->GetTimeSeconds();   // WorldSubsystem은 항상 valid world 보유

	// 빈 슬롯 있으면 즉시 grant.
	if (TokenHolders.Num() < GKDMaxAttackers)
	{
		TokenHolders.Add({ Enemy, Now });
		return true;
	}

	// 꽉 참 — hold-time 초과 보유자 중 가장 오래된 1명 회수 후 grant(로테이션).
	int32 OldestIdx = INDEX_NONE;
	float OldestGrant = Now;
	for (int32 i = 0; i < TokenHolders.Num(); ++i)
	{
		if (Now - TokenHolders[i].GrantTime >= GKDTokenHoldTime && TokenHolders[i].GrantTime <= OldestGrant)
		{
			OldestGrant = TokenHolders[i].GrantTime;
			OldestIdx = i;
		}
	}
	if (OldestIdx != INDEX_NONE)
	{
		TokenHolders[OldestIdx] = { Enemy, Now };
		return true;
	}

	return false;   // 슬롯 꽉 차고 아무도 hold-time 미초과 → 대기
}

void UKDEncounterSubsystem::ReturnToken(AKDEnemyBaseCharacter* Enemy)
{
	if (!Enemy) return;
	TokenHolders.RemoveAll([Enemy](const FTokenSlot& Slot) { return Slot.Holder.Get() == Enemy; });
}
