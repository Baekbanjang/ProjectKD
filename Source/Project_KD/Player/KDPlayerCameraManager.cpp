// Fill out your copyright notice in the Description page of Project Settings.


#include "KDPlayerCameraManager.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"

AKDPlayerCameraManager::AKDPlayerCameraManager()
{
	ViewPitchMin = -89.0f;
	ViewPitchMax = 45.0f;
	
	DefaultFOV = 75.0f;
}


void AKDPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	// 카메라가 벽 등으로 가까워지면 FOV 자동 좁아짐(시네마틱).
	// 평상시(TargetArmLength=382) -> FOV 75, 벽 밀착 -> FOV 40 근접.
	if (!FovByDistanceCurve || !PCOwner || !PCOwner->GetPawn())
	{
		return;
	}

	// 시네마틱(처형/페이탈 등) 중엔 게임플레이 카메라가 손 떼고 시퀀스/시네카메라에 양보
	if (UAbilitySystemComponent* CineASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PCOwner->GetPawn()))
	{
		if (CineASC->HasMatchingGameplayTag(GameplayTags::State_Camera_Cinematic))
		{
			return; 
		}
	}

	// FOV 펀치 전이 델타
	float PunchDelta = 0.f;
	if (FovPunchCurve && FovPunchElapsed >= 0.f)
	{
		float MinT, MaxT;
		FovPunchCurve->GetTimeRange(MinT, MaxT);
		PunchDelta = FovPunchCurve->GetFloatValue(FovPunchElapsed) * FovPunchScale;
		FovPunchElapsed += DeltaTime;
		if (FovPunchElapsed > MaxT) FovPunchElapsed = -1.f; // 끝 → 비활성
	}
	
	// 락온 중엔 동적 FOV(거리별 좁힘) 끄고 기본 FOV 유지 — 높은 적 올려다볼 때 붐 충돌로 FOV가 확 좁아져 시야 막히는 것 방지.
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PCOwner->GetPawn()))
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn))
		{
			ViewTarget.POV.FOV = DefaultFOV + PunchDelta;
			return;
		}
	}

	// 캐릭터 위치 — 거리 측정 기준점.
	const FVector PawnLoc = PCOwner->GetPawn()->GetActorLocation();

	// 캐릭터 - 카메라 실제 거리 (SpringArm 벽 충돌 시 짧아짐).
	const float CameraDist = FVector::Distance(PawnLoc, ViewTarget.POV.Location);

	// 거리 -> 커브 평가 -> 원하는 FOV 추출.
	const float DesiredFOV = FovByDistanceCurve->GetFloatValue(CameraDist);
	
	// CameraComponent 기본 FOV를 동적 값으로 override.
	ViewTarget.POV.FOV = DesiredFOV + PunchDelta;
}

void AKDPlayerCameraManager::TriggerFovPunch(float Scale)
{
    FovPunchElapsed = 0.f; // 펀치 시작
	FovPunchScale = Scale;
}
