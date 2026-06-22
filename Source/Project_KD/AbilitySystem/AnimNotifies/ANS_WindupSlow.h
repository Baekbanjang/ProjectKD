#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WindupSlow.generated.h"

// Drag onto an enemy attack Montage's wind-up region to stretch the telegraph,
// giving the player a fair reaction window (asymmetric combat: only enemies telegraph).
// Begin → Montage_SetPlayRate(SlowRate). End → Montage_SetPlayRate(RestoreRate).
// Wide region + moderate SlowRate = uniform slow. Narrow region at the apex + low
// SlowRate = "apex hold" (reads as charging). Keep this region OUT of the forward
// swing so root-motion lunge distance stays full-speed.
UCLASS(meta = (DisplayName = "Windup Slow (Telegraph)"))
class PROJECT_KD_API UANS_WindupSlow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_WindupSlow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	// Play rate during the region. Min 0.05 (never 0 — pausing deadlocks NotifyEnd).
	UPROPERTY(EditAnywhere, Category = "Windup", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float SlowRate = 0.4f;

	// Rate restored on exit. Must match the montage's base play rate (GA plays at 1.0).
	UPROPERTY(EditAnywhere, Category = "Windup", meta = (ClampMin = "0.05", ClampMax = "4.0"))
	float RestoreRate = 1.0f;
};
