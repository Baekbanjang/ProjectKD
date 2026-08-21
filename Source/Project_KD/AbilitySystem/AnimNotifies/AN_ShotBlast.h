// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ShotBlast.generated.h"

// 콤보 속 총 발사 프레임 노티 — Event.Montage.ShotBlast 전송
UCLASS(meta = (DisplayName = "Shot Blast"))
class PROJECT_KD_API UAN_ShotBlast : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category = "Shot", meta = (DisplayName = "히트스톱 끄기"))
	bool bIgnoreHitStop = false;

	// 총구 방향 발사 유무 — 회전 연사
	UPROPERTY(EditAnywhere, Category = "Shot", meta = (DisplayName = "총구 방향으로 발사"))
	bool bUseMuzzleDirection = false;

	// 콘 원점 = 총구 유무 — 공중 자세 타는 캡슐(지면)과 몸이 어긋남
	UPROPERTY(EditAnywhere, Category = "Shot", meta = (DisplayName = "총구에서 발사"))
	bool bUseMuzzleOrigin = false;

	// 발사 각도
	UPROPERTY(EditAnywhere, Category = "Shot",
		meta = (ClampMin = "0.0", ClampMax = "180.0", DisplayName = "각도 예외 (0 = GA 값)"))
	float ShotHalfAngleOverride = 0.f;

	UPROPERTY(EditAnywhere, Category = "Shot",
		meta = (ClampMin = "0.0", ClampMax = "5.0", DisplayName = "데미지 배수 (0 = GA 값)"))
	float ShotDamageMultiplierOverride = 0.f;

	UPROPERTY(EditAnywhere, Category = "Shot",
	meta = (ClampMin = "0.0", ClampMax = "5.0", DisplayName = "넉백 배수 (0 = GA 값)"))
	float ShotKnockbackMultiplierOverride = 0.f;

	// 자동 조준 탐색 각도
	UPROPERTY(EditAnywhere, Category = "Shot",
		meta = (ClampMin = "0.0", ClampMax = "360.0", DisplayName = "조준 탐색 각도 (0 = GA 값)"))
	float AutoAimConeAngleOverride = 0.f;

	// 몸 정면 기준 조준 한계각
	UPROPERTY(EditAnywhere, Category = "Shot",
		meta = (ClampMin = "0.0", ClampMax = "180.0", DisplayName = "몸 조준 한계각 (0 = GA 값)"))
	float BodyAimLimitAngleOverride = 0.f;
};
