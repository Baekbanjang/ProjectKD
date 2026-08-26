#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "KDAnimNotifyState_CancelWindow.generated.h"

// D8 P2 — drop on Montage recovery phase to grant State.Combat.CanCancel during the window.
// GAs that wish to be cancelable mid-action gate themselves via this tag.
// Begin: AddLooseGameplayTag(CancelTag) on owner ASC.
// End:   RemoveLooseGameplayTag(CancelTag).
UCLASS(meta = (DisplayName = "Cancel Window"))
class PROJECT_KD_API UKDAnimNotifyState_CancelWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UKDAnimNotifyState_CancelWindow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Cancel")
	FGameplayTag CancelTag;
};
