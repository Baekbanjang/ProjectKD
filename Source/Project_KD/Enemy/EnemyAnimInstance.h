#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

class UHitFeedbackComponent;

// BoneShake 오프셋을 C++에서 계산. AnimBP child는 Modify Bone Translation 핀만 연결하면 됨.
UCLASS()
class PROJECT_KD_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "HitFeedback")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "HitFeedback")
	FVector Spine01Offset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "HitFeedback")
	FVector Spine02Offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PelvisWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Spine01Weight = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Spine02Weight = 0.4f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.f;

	// -180~180, facing 대비 이동방향 (strafe BlendSpace X축). kiting 뒷걸음 = Direction≈±180.
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bInCombat = false;

	// 타겟 인지 여부(블랙보드 TargetActor 유무). bInCombat=근접 strafe거리(400) 게이트와 별개 —
	// 활은 거리 무관 인지 즉시 활 듦. ABP Aim 상태 진입/이탈 게이트용.
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHasTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float CombatEnterRange = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float CombatExitRange = 500.f;

	// 이동 속도 — bInCombat 따라 MaxWalkSpeed 전환(애니 속도와 실이동을 맞춰 foot sliding 방지).
	// Combat(검 듦 strafe, BS_Combat 최대치)=느림, Travel(검 내림 추격, BS_Travel 최대치)=빠름. ABP child 튜닝.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float CombatWalkSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float TravelWalkSpeed = 500.f;

	// 타겟 없음(패트롤) = 걷기. BS 걷기 샘플 속도와 일치시켜야 발이 안 미끄러짐. ABP child 튜닝.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float PatrolWalkSpeed = 200.f;


private:
	UPROPERTY(Transient)
	TObjectPtr<UHitFeedbackComponent> CachedHitFeedback;
};
