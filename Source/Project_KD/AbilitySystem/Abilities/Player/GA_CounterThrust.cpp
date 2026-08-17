// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_CounterThrust.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "Combat/LockOnComponent.h"

UGA_CounterThrust::UGA_CounterThrust()
{
	AbilityTags.AddTag(GameplayTags::Ability_Player_CounterThrust);
}

void UGA_CounterThrust::OnActivated()
{
	Super::OnActivated();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		ASC->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_CounterThrust, CueParams);
	}


	AActor* PC = GetAvatarActorFromActorInfo();
	ULockOnComponent* LockOn = GetLockOnComponentFromActorInfo();
	if (!IsValid(PC) || !LockOn) return;

	// 락온 타겟 우선, 없으면 자동 탐색. 둘 다 없으면 제자리 찌르기.
	AActor* Target = LockOn->GetLockedTarget();
	if (!Target) Target = LockOn->FindBestTarget();
	if (!Target) return;

	FVector ToTarget = Target->GetActorLocation() - PC->GetActorLocation();
	ToTarget.Z = 0.f;
	const float Dist = ToTarget.Size();
	if (Dist > MaxDashRange) return; // 너무 멀면 대시 생략, 찌르기만

	const FVector Dir = ToTarget.GetSafeNormal();
	if (Dir.IsNearlyZero()) return;

	UMotionWarpingComponent* Warp = PC->FindComponentByClass<UMotionWarpingComponent>();
	if (!ensureMsgf(Warp, TEXT("[KD] CounterThrust: 플레이어에 MotionWarpingComponent 없음 — 찌르기만 진행")))
		return;

	// 적 앞 WarpStopDistance 지점까지 애니 이동량을 보정.
	// Dir = 적 방향, Dist = 적까지 거리. 겹치지 않게 그만큼 덜 가서 정지
	const FVector WarpLoc = PC->GetActorLocation() + Dir * FMath::Max(Dist - WarpStopDistance, 0.f);

	// 그 지점/방향을 워핑 동기점에 등록 -> 몽타주 루트모션이 WarpLoc에 정확히 착지하도록 늘어남
	Warp->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, WarpLoc, Dir.Rotation());


	// 해당 모션 워핑 로직
	// 몽타주 안에 같은 이름("CounterTarget")의 Motion Warping 노티가 있으면,
	// 그 구간 동안 루트모션이 자동으로 늘어나/줄어서 캐릭터가 WarpLoc에 도착하고 Dir.Rotation()(적 쳐다보는 방향)으로 회전
}
