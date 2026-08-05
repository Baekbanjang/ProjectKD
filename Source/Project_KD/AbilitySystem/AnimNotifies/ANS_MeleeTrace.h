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

	// ── 아래는 전부 "이 창만 예외" 설정. 비워두면 GA 기본값이 그대로 쓰인다. ──
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (DisplayName = "시작 소켓", ToolTip = "비우면 GA의 Start Socket 사용. 발차기 창이면 foot_r 등"))
	FName StartSocketOverride = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (DisplayName = "끝 소켓", ToolTip = "비우면 GA의 End Socket 사용"))
	FName EndSocketOverride = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)", meta = (ClampMin = "0.0",
		DisplayName = "캡슐 반경 (0 = GA 값)", ToolTip = "0이면 GA의 Capsule Radius 사용. 판정이 없어지는 게 아님"))
	float CapsuleRadiusOverride = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (DisplayName = "판정 모양 바꾸기", ToolTip = "찌르기 한 방만 끝점 판정으로 돌릴 때 체크"))
	bool bOverrideTraceMode = false;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (EditCondition = "bOverrideTraceMode", DisplayName = "└ 바꿀 모양"))
	ETraceMode TraceModeOverride = ETraceMode::Sweep;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (DisplayName = "판정 대상 바꾸기 (발차기)", ToolTip = "칼이 아니라 발/주먹으로 때리는 창일 때 체크"))
	bool bOverrideMeshSource = false;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (EditCondition = "bOverrideMeshSource", DisplayName = "└ 바꿀 대상"))
	ETraceMeshSource MeshSourceOverride = ETraceMeshSource::OwnerBody;
	
	UPROPERTY(EditAnywhere, Category = "예외 (비우면 GA 값 사용)",
		meta = (DisplayName = "히트스톱 끄기"))
	bool bIgnoreHitStop = false;
};
