#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_WindupSlow.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy/KDEnemyBaseCharacter.h"

static float GetEffectiveSlowRate(USkeletalMeshComponent* MeshComp, float SlowRate)
{
	float Multiplier = 1.0f;
	if (AKDEnemyBaseCharacter* Char = Cast<AKDEnemyBaseCharacter>(MeshComp->GetOwner()))
		Multiplier = Char->GetTelegraphSlowMultiplier();
	return FMath::Clamp(SlowRate * Multiplier, 0.05f, 1.0f);
}

UKDAnimNotifyState_WindupSlow::UKDAnimNotifyState_WindupSlow()
{
}

void UKDAnimNotifyState_WindupSlow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance) return;

	if (UAnimMontage* Montage = AnimInstance->GetCurrentActiveMontage())
	{
		AnimInstance->Montage_SetPlayRate(Montage, GetEffectiveSlowRate(MeshComp, SlowRate));
	}
}

void UKDAnimNotifyState_WindupSlow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance) return;

	// If the windup was interrupted (e.g. player staggered the enemy), a different montage may be
	// active. Only restore when the current montage is still at OUR SlowRate — otherwise we'd stomp
	// the play rate of whatever montage took over (e.g. a stagger reaction running at its own rate).
	if (UAnimMontage* Montage = AnimInstance->GetCurrentActiveMontage())
	{
		if (FMath::IsNearlyEqual(AnimInstance->Montage_GetPlayRate(Montage), GetEffectiveSlowRate(MeshComp, SlowRate)))
		{
			AnimInstance->Montage_SetPlayRate(Montage, RestoreRate);
		}
	}
}

FString UKDAnimNotifyState_WindupSlow::GetNotifyName_Implementation() const
{
	return TEXT("WindupSlow");
}
