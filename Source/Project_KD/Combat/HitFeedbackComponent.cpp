#include "Combat/HitFeedbackComponent.h"

UHitFeedbackComponent::UHitFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHitFeedbackComponent::TriggerBoneShake()
{
	TriggerBoneShakeParams(ShakeIntensity, ShakeDuration);
}

void UHitFeedbackComponent::TriggerBoneShakeParams(float Intensity, float Duration)
{
	ActiveIntensity = Intensity;
	ActiveDuration = FMath::Max(0.05f, Duration);
	ShakeTimeRemaining = ActiveDuration;
	CurrentShakeAlpha = 1.0f;
}

void UHitFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShakeTimeRemaining > 0.0f)
	{
		ShakeTimeRemaining = FMath::Max(0.0f, ShakeTimeRemaining - DeltaTime);
		CurrentShakeAlpha = ShakeTimeRemaining / ActiveDuration;
	}
	else if (CurrentShakeAlpha != 0.0f)
	{
		CurrentShakeAlpha = 0.0f;
	}
}
