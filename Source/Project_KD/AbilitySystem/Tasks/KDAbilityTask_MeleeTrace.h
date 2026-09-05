#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "KDAbilityTask_MeleeTrace.generated.h"

class UMeshComponent;
struct FHitResult;
struct FCollisionQueryParams;
struct FCollisionObjectQueryParams;

// Sweep    : 무기 축 전체를 캡슐로 prev→cur 스윕 — 넓게 잡힘, 칼몸 어디든 판정
// TipLine  : 칼끝만 prev -> cur 라인 트레이스 — 얇은 무기용, 칼몸 판정 X
// ArcSweep : Sweep + 베지어 경로 — 캡슐이 곡선을 따라 이동
// ArcTri   : 베지어 경로 + 칸마다 선 격자 — 두께 0, SB Triangle-Hitbox 방식

UENUM(BlueprintType)
enum class ETraceMode : uint8
{
	Sweep    UMETA(DisplayName = "Capsule Sweep (whole shaft)"),
	TipLine  UMETA(DisplayName = "Tip LineTrace (thin weapons)"),
	ArcSweep  UMETA(DisplayName = "Arc Capsule Sweep (curved)"),
	ArcTri    UMETA(DisplayName = "Arc Triangle Lines (SB style)")
};

// 판정 출처
UENUM(BlueprintType)
enum class ETraceMeshSource : uint8
{
	Weapon    UMETA(DisplayName = "Weapon Mesh"),
	OwnerBody UMETA(DisplayName = "Owner Body Mesh"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTraceHitDelegate, const FHitResult&, Hit);

// StartSocket↔EndSocket 축을 따라 prev→cur 구간 판정. 모드 4종
// 서브스텝 보간으로 고속 스윙 터널링 방지. 태스크 수명 동안 액터당 OnHit 1회
UCLASS()
class PROJECT_KD_API UKDAbilityTask_MeleeTrace : public UAbilityTask
{
	GENERATED_BODY()

public:
	UKDAbilityTask_MeleeTrace(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UKDAbilityTask_MeleeTrace* MeleeTrace(
		UGameplayAbility* OwningAbility,
		UMeshComponent* WeaponMesh,
		FName StartSocket,
		FName EndSocket,
		ETraceMode Mode = ETraceMode::TipLine,
		float CapsuleRadius = 3.0f,
		bool bDrawDebug = true,
		float ArcBulge = 1.0f,
		int32 TraceSegments = 3);

	UPROPERTY(BlueprintAssignable)
	FWeaponTraceHitDelegate OnHit;

	// 한 구간에 대한 판정 로직 - TickTask가 매 틱 호출
	void TraceOnce();

protected:
	// GAS framework callbacks — base UGameplayTask declares these protected; keep visibility matched.
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	// 모드별 판정
	void TraceTipLine(const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& Params,
		const FVector& CurStart, const FVector& CurEnd);
	
	void TraceSweep(const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& Params,
		const FVector& CurStart, const FVector& CurEnd);
	
	void TraceArc(const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& Params,
		const FVector& CurStart, const FVector& CurEnd);
	
	// 히트 처리
	void ProcessHits(const TArray<FHitResult>& Hits, const FCollisionQueryParams& Params);
	
	// 벽 차단 유무
	bool IsWallBlocking(const FVector& End, const FCollisionQueryParams& Params) const;
	
	UPROPERTY()
	TObjectPtr<UMeshComponent> WeaponMesh;

	FName StartSocket = NAME_None;
	FName EndSocket = NAME_None;
	ETraceMode Mode = ETraceMode::TipLine;
	float CapsuleRadius = 3.0f;
	bool bDrawDebug = true;
	float ArcBulge = 1.0f;
	int32 TraceSegments = 3;

	bool bHasPrevFrame = false;
	FVector PrevStart = FVector::ZeroVector;
	FVector PrevEnd = FVector::ZeroVector;

	// Per-task once-per-actor filter so a single swing doesn't multi-hit the same target.
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
};
