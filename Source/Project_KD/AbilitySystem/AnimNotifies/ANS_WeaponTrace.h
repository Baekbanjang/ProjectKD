#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tasks/AT_WeaponTrace.h" // ETraceMode
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrace.generated.h"

// 몽타주 타임라인에 배치해 무기 위험 구간을 표시.
// Begin → Event.Montage.TraceBegin 발신(GA가 UAT_WeaponTrace 시작). End → TraceEnd.
// Per-window 오버라이드(below): empty/zero = GA 기본값 상속. Payload.OptionalObject=this로 동기 전달.
UCLASS(meta = (DisplayName = "Weapon Trace Window"))
class PROJECT_KD_API UANS_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_WeaponTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	// NAME_None = inherit GA's StartSocket/EndSocket.
	UPROPERTY(EditAnywhere, Category = "Trace|Override")
	FName StartSocketOverride = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Trace|Override")
	FName EndSocketOverride = NAME_None;

	// 0 = inherit GA's CapsuleRadius.
	UPROPERTY(EditAnywhere, Category = "Trace|Override", meta = (ClampMin = "0.0"))
	float CapsuleRadiusOverride = 0.f;

	// Check to swap trace mode for just this window (e.g. TipLine tip-only on a thrust within a Sweep combo).
	UPROPERTY(EditAnywhere, Category = "Trace|Override")
	bool bOverrideTraceMode = false;

	UPROPERTY(EditAnywhere, Category = "Trace|Override", meta = (EditCondition = "bOverrideTraceMode"))
	ETraceMode TraceModeOverride = ETraceMode::Sweep;
};
