#include "AbilitySystem/Tasks/AT_MeleeTrace.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAT_MeleeTrace::UAT_MeleeTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UAT_MeleeTrace* UAT_MeleeTrace::MeleeTrace(
	UGameplayAbility* OwningAbility,
	USkeletalMeshComponent* InWeaponMesh,
	FName InStartSocket,
	FName InEndSocket,
	ETraceMode InMode,
	float InCapsuleRadius,
	bool bInDrawDebug)
{
	UAT_MeleeTrace* Task = NewAbilityTask<UAT_MeleeTrace>(OwningAbility);
	Task->WeaponMesh = InWeaponMesh;
	Task->StartSocket = InStartSocket;
	Task->EndSocket = InEndSocket;
	Task->Mode = InMode;
	Task->CapsuleRadius = InCapsuleRadius;
	Task->bDrawDebug = bInDrawDebug;
	return Task;
}

void UAT_MeleeTrace::Activate()
{
	Super::Activate();

	if (!IsValid(WeaponMesh))
	{
		EndTask();
		return;
	}

	bHasPrevFrame = false;
	AlreadyHitActors.Reset();
}

void UAT_MeleeTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!IsValid(WeaponMesh))
	{
		EndTask();
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector CurStart = WeaponMesh->GetSocketLocation(StartSocket);
	const FVector CurEnd = WeaponMesh->GetSocketLocation(EndSocket);

	// Seed prev-frame on first tick so we never sweep from origin.
	if (!bHasPrevFrame)
	{
		PrevStart = CurStart;
		PrevEnd = CurEnd;
		bHasPrevFrame = true;
		return;
	}

	AActor* Owner = GetAvatarActor();
	if (!Owner)
	{
		EndTask();
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), false, Owner);
	Params.AddIgnoredActor(Owner);
	Params.bReturnPhysicalMaterial = false;

	// Channel response 무시하고 Pawn ObjectType actor 직접 매칭. GASP 캐릭터 capsule이
	// Pawn channel = Ignore여도 Object Type이 Pawn이면 잡힘.
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel2); // Destructible (DefaultEngine.ini)

	// LoS wall check: returns true only if static world geometry blocks Origin→End.
	// Querying the WorldStatic OBJECT type (not Visibility channel) means pawns never occlude —
	// a target standing behind another enemy still gets hit, while real walls still block.
	FCollisionObjectQueryParams WallParams;
	WallParams.AddObjectTypesToQuery(ECC_WorldStatic);
	auto IsWallBlocking = [&](const FVector& Origin, const FVector& End) -> bool
	{
		return World->LineTraceTestByObjectType(Origin, End, WallParams, Params);
	};

	// Shared hit handling for both trace modes: skip self/wall-occluded/already-hit, then broadcast once.
	auto ProcessHits = [&](const TArray<FHitResult>& Hits)
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == Owner) continue;
			if (AlreadyHitActors.Contains(HitActor)) continue;
			if (IsWallBlocking(Owner->GetActorLocation(), Hit.ImpactPoint)) continue;
			AlreadyHitActors.Add(HitActor);
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnHit.Broadcast(Hit);
			}
		}
	};

	bool bAnyHit = false;

	if (Mode == ETraceMode::TipLine)
	{
		// Trace only the spear tip's swept path (PrevEnd → CurEnd). Stationary tip → zero-length
		// trace → no hit. Single LineTrace covers the whole tick: tip motion is linear within a
		// frame so SubSteps add nothing for a line. Thin-weapon SB tone.
		TArray<FHitResult> Hits;
		bAnyHit = World->LineTraceMultiByObjectType(Hits, PrevEnd, CurEnd, ObjectParams, Params);
		if (bAnyHit)
		{
			ProcessHits(Hits);
		}

		if (bDrawDebug)
		{
			DrawDebugLine(World, PrevEnd, CurEnd,
				bAnyHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
		}
	}
	else // Sweep
	{
		// SubSteps interpolation: split prev→cur travel into 1~8 sub-sweeps to avoid 30fps tunneling
		// on fast swings. StepDist slightly under capsule diameter ensures no gap.
		const FVector PrevMid = (PrevStart + PrevEnd) * 0.5f;
		const FVector CurMid  = (CurStart + CurEnd) * 0.5f;
		const float TravelDist = (CurMid - PrevMid).Size();
		constexpr float StepDist = 5.0f; // cm
		const int32 SubSteps = FMath::Clamp(FMath::CeilToInt(TravelDist / StepDist), 1, 8);

		for (int32 Step = 0; Step < SubSteps; ++Step)
		{
			const float A0 = static_cast<float>(Step) / SubSteps;
			const float A1 = static_cast<float>(Step + 1) / SubSteps;

			const FVector S0 = FMath::Lerp(PrevStart, CurStart, A0);
			const FVector E0 = FMath::Lerp(PrevEnd,   CurEnd,   A0);
			const FVector S1 = FMath::Lerp(PrevStart, CurStart, A1);
			const FVector E1 = FMath::Lerp(PrevEnd,   CurEnd,   A1);

			const FVector Mid0 = (S0 + E0) * 0.5f;
			const FVector Mid1 = (S1 + E1) * 0.5f;
			const FVector Axis1 = E1 - S1;
			const float HalfH1 = FMath::Max(Axis1.Size() * 0.5f, CapsuleRadius);
			const FQuat Rot1 = FRotationMatrix::MakeFromZ(Axis1).ToQuat();

			TArray<FHitResult> Hits;
			const bool bHit = World->SweepMultiByObjectType(
				Hits, Mid0, Mid1, Rot1, ObjectParams,
				FCollisionShape::MakeCapsule(CapsuleRadius, HalfH1), Params);

			bAnyHit |= bHit;

			if (bHit)
			{
				ProcessHits(Hits);
			}

			// 서브스텝마다 실제 스윕 캡슐을 그림 — 저프레임에서 틱당 1개만 그리던 거짓 불균일 제거,
			// 보간 공백이 실제로 벌어지는지 눈으로 확인 가능.
			if (bDrawDebug)
			{
				DrawDebugCapsule(World, Mid1, HalfH1, CapsuleRadius, Rot1,
					bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
			}
		}
	}

	PrevStart = CurStart;
	PrevEnd = CurEnd;
}

void UAT_MeleeTrace::OnDestroy(bool bInOwnerFinished)
{
	AlreadyHitActors.Reset();
	Super::OnDestroy(bInOwnerFinished);
}
