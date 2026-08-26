#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KDAnimNotifyState_TelegraphWindow.generated.h"

// Place in the montage windup region (before TraceBegin) to control telegraph timing.
// NotifyBegin → AddGameplayCue(TelegraphCueTag)  → GCN_EnemyTelegraph spawns the VFX.
// NotifyEnd   → RemoveGameplayCue(TelegraphCueTag) → GCN_EnemyTelegraph stops the VFX.
// GA_EnemyBasicAttack::OnCleanup keeps its own RemoveGameplayCue as a cancellation safety net
// in case the ability is interrupted before NotifyEnd fires.
UCLASS(meta = (DisplayName = "Telegraph Window"))
class PROJECT_KD_API UKDAnimNotifyState_TelegraphWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UKDAnimNotifyState_TelegraphWindow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	// Leaf tag passed to AddGameplayCue. Must match TelegraphCueTag set in GA_EnemyBasicAttack CDO.
	// e.g. GameplayCue.Enemy.Telegraph.Unblockable
	UPROPERTY(EditAnywhere, Category = "Telegraph", meta = (Categories = "GameplayCue.Enemy.Telegraph"))
	FGameplayTag TelegraphCueTag;
};
