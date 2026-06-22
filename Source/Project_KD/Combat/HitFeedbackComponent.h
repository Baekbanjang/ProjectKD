#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitFeedbackComponent.generated.h"

// BoneShake 타이밍 제어. AnimBP가 CurrentShakeAlpha를 읽어 Spine/Pelvis Modify Bone 오프셋 스케일.
// GameplayCue가 피격 시 TriggerBoneShake 호출.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECT_KD_API UHitFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitFeedbackComponent();

	// AnimBP가 오프셋을 적용할 본 목록.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BoneShake")
	TArray<FName> ShakeBones = { TEXT("spine_01"), TEXT("spine_02"), TEXT("pelvis") };

	// Peak random offset in cm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BoneShake", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float ShakeIntensity = 5.0f;

	// Total shake duration (decays linearly to 0).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BoneShake", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float ShakeDuration = 0.2f;

	// Read by AnimBP each frame. 1.0 = peak intensity, 0.0 = no shake. Linear decay.
	UPROPERTY(BlueprintReadOnly, Category = "BoneShake")
	float CurrentShakeAlpha = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "BoneShake")
	void TriggerBoneShake();

	UFUNCTION(BlueprintCallable, Category = "BoneShake")
	void TriggerBoneShakeParams(float Intensity, float Duration);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float ShakeTimeRemaining = 0.0f;
	float ActiveDuration = 0.0f;
	float ActiveIntensity = 0.0f;
};
