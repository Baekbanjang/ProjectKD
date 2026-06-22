#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WeaponTrace.generated.h"

class USkeletalMeshComponent;

// Sweep    : capsule prev→cur sweep along whole weapon axis (broad, hits along entire shaft)
// TipLine  : line trace from prev-tip to cur-tip only (thin weapons; ignores shaft body — SB tone)
UENUM(BlueprintType)
enum class ETraceMode : uint8
{
	Sweep    UMETA(DisplayName = "Capsule Sweep (whole shaft)"),
	TipLine  UMETA(DisplayName = "Tip LineTrace (thin weapons)")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTraceHitDelegate, const FHitResult&, Hit);

// StartSocket↔EndSocket 축을 따라 prev→cur 프레임 궤적을 캡슐 스윕.
// SubSteps(1~8) 보간으로 고속 스윙 터널링 방지. 태스크 수명 동안 액터당 OnHit 1회.
UCLASS()
class PROJECT_KD_API UAT_WeaponTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WeaponTrace(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAT_WeaponTrace* WeaponTrace(
		UGameplayAbility* OwningAbility,
		USkeletalMeshComponent* WeaponMesh,
		FName StartSocket,
		FName EndSocket,
		ETraceMode Mode = ETraceMode::TipLine,
		float CapsuleRadius = 3.0f,
		bool bDrawDebug = true);

	UPROPERTY(BlueprintAssignable)
	FWeaponTraceHitDelegate OnHit;

protected:
	// GAS framework callbacks — base UGameplayTask declares these protected; keep visibility matched.
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	FName StartSocket = NAME_None;
	FName EndSocket = NAME_None;
	ETraceMode Mode = ETraceMode::TipLine;
	float CapsuleRadius = 3.0f;
	bool bDrawDebug = true;

	bool bHasPrevFrame = false;
	FVector PrevStart = FVector::ZeroVector;
	FVector PrevEnd = FVector::ZeroVector;

	// Per-task once-per-actor filter so a single swing doesn't multi-hit the same target.
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
};
