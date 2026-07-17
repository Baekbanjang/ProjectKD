#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tasks/AT_MeleeTrace.h" // ETraceMode
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_MeleeTrace.generated.h"

// 몽타주 타임라인에 배치해 무기 위험 구간을 표시.
// Begin → Event.Montage.TraceBegin 발신(GA가 UAT_MeleeTrace 시작). End → TraceEnd.
// Per-window 오버라이드(below): empty/zero = GA 기본값 상속. Payload.OptionalObject=this로 동기 전달.
UCLASS(meta = (DisplayName = "Melee Trace Window"))
class PROJECT_KD_API UANS_MeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_MeleeTrace();

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

	// 이 창에서만 트레이스 모드를 바꿀 때 체크 (예: Sweep 콤보 중 찌르기 한 방만 TipLine 끝점 판정)
	UPROPERTY(EditAnywhere, Category = "Trace|Override")
	bool bOverrideTraceMode = false;

	// 이 창의 트레이스를 다른 메쉬로 돌릴 때 체크 (예: 무기 콤보 중 킥 한 방만 OwnerBody 본체 메쉬로 판정)
	UPROPERTY(EditAnywhere, Category = "Trace|Override", meta = (EditCondition = "bOverrideTraceMode"))
	ETraceMode TraceModeOverride = ETraceMode::Sweep;

	
	UPROPERTY(EditAnywhere, Category = "Trace|Override")
	bool bOverrideMeshSource = false;

	UPROPERTY(EditAnywhere, Category = "Trace|Override", meta = (EditCondition = "bOverrideMeshSource"))
	ETraceMeshSource MeshSourceOverride = ETraceMeshSource::OwnerBody;
};
