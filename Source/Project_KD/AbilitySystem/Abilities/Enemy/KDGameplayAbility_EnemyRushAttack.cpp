// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/Enemy/KDGameplayAbility_EnemyRushAttack.h"

#include "AIController.h"
#include "KDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h"

static constexpr float RushNearSkipThreshold = 50.f;

void UKDGameplayAbility_EnemyRushAttack::OnActivated()
{
	// base: braking glide 제거(plant) + 전조 큐. 돌진은 루트모션이 이동을 전담하므로 plant OK.
	Super::OnActivated();

	// 발동 순간 1회 조준(폴백). 재조준 노티가 없거나 늦으면 최소한 이 값으로 돌진.
	UpdateWarpTarget();

	// 돌진 윈도우 시작에 둔 AN_SendGameplayEvent(Event.Rush.Warp)가 발화하면 현재 위치로 재조준.
	UAbilityTask_WaitGameplayEvent* WarpListener = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Rush_Warp, nullptr, false, true);
	WarpListener->EventReceived.AddDynamic(this, &UKDGameplayAbility_EnemyRushAttack::OnRushWarpEvent);
	WarpListener->ReadyForActivation();

	// 윈드업 중 플레이어 쪽으로 회전 추적 (워프 직전 ClearFocus로 해제).
	APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (Avatar)
	{
		if (AAIController* AICon = Cast<AAIController>(Avatar->GetController()))
		{
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(Avatar, 0))
				AICon->SetFocus(PlayerPawn);
		}
	}
}

void UKDGameplayAbility_EnemyRushAttack::OnRushWarpEvent(FGameplayEventData Payload)
{
	// 워프 순간 회전 고정 → 돌진 방향 확정.
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn)
	{
		if (AAIController* AICon = Cast<AAIController>(AvatarPawn->GetController()))
			AICon->ClearFocus(EAIFocusPriority::Gameplay);
	}

	UpdateWarpTarget();
}

void UKDGameplayAbility_EnemyRushAttack::OnCleanup(bool bWasCancelled)
{
	// 캔슬 경로 포함 — AI 포커스 반드시 해제.
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn)
	{
		if (AAIController* AICon = Cast<AAIController>(AvatarPawn->GetController()))
			AICon->ClearFocus(EAIFocusPriority::Gameplay);
	}

	Super::OnCleanup(bWasCancelled);
}

void UKDGameplayAbility_EnemyRushAttack::UpdateWarpTarget()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	UMotionWarpingComponent* Warp = Avatar->FindComponentByClass<UMotionWarpingComponent>();
	if (!Warp)
	{
		// 컴포넌트 미설정 → warp 없이 제자리 몽타주. BP_AxeElite에 MotionWarping 컴포넌트 추가 필요.
		ensureMsgf(false, TEXT("[KD] %s: no MotionWarpingComponent on %s — add it to BP_AxeElite."),
			*GetName(), *Avatar->GetName());
		return;
	}

	const APawn* Target = UGameplayStatics::GetPlayerPawn(Avatar, 0);
	if (!Target) return;

	const FVector AvatarLoc = Avatar->GetActorLocation();
	FVector ToTarget = Target->GetActorLocation() - AvatarLoc;
	ToTarget.Z = 0.f; // 수평 돌진(지면 유지) — 워프가 캐릭터를 위/아래로 끌지 않게.
	const FVector Dir = ToTarget.GetSafeNormal();
	if (Dir.IsNearlyZero()) return;

	const float RawDist = ToTarget.Size() - RushStopDistance;

	// 근접 시 워프 생략 — 0거리 압축 버벅임 제거.
	if (RawDist <= RushNearSkipThreshold)
	{
		Warp->RemoveWarpTarget(WarpTargetName);
		return;
	}

	// 상한 클램프: MaxRushWarpDistance 초과 워프 금지.
	const float ClampedDist = FMath::Min(FMath::Max(RawDist, 0.f), MaxRushWarpDistance);
	const FVector WarpLoc = AvatarLoc + Dir * ClampedDist;
	Warp->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, WarpLoc, Dir.Rotation());
}
