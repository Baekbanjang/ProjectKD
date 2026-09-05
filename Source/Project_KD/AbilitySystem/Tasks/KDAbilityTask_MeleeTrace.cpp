#include "AbilitySystem/Tasks/KDAbilityTask_MeleeTrace.h"

#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	// 서브스텝 간격 — 칸 하나의 목표 이동거리
	constexpr float StepDist = 5.0f;

	// 2차 베지어 — T=0 이면 P0, T=1 이면 P2 를 정확히 지난다. P1 은 곡선을 당기는 제어점
	FORCEINLINE FVector Bezier2(const FVector& P0, const FVector& P1, const FVector& P2, float T)
	{
		const float U = 1.f - T;
		return U * U * P0 + 2.f * U * T * P1 + T * T * P2;
	}
}

UKDAbilityTask_MeleeTrace::UKDAbilityTask_MeleeTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UKDAbilityTask_MeleeTrace* UKDAbilityTask_MeleeTrace::MeleeTrace(
	UGameplayAbility* OwningAbility,
	UMeshComponent* InWeaponMesh,
	FName InStartSocket,
	FName InEndSocket,
	ETraceMode InMode,
	float InCapsuleRadius,
	bool bInDrawDebug,
	float InArcBulge,
	int32 InTraceSegments,
	int32 InMaxSubSteps)
{
	UKDAbilityTask_MeleeTrace* Task = NewAbilityTask<UKDAbilityTask_MeleeTrace>(OwningAbility);
	Task->WeaponMesh = InWeaponMesh;
	Task->StartSocket = InStartSocket;
	Task->EndSocket = InEndSocket;
	Task->Mode = InMode;
	Task->CapsuleRadius = InCapsuleRadius;
	Task->bDrawDebug = bInDrawDebug;
	Task->ArcBulge = InArcBulge;
	Task->TraceSegments = FMath::Max(1, InTraceSegments);
	Task->MaxSubSteps = FMath::Max(1, InMaxSubSteps);
	return Task;
}

void UKDAbilityTask_MeleeTrace::Activate()
{
	Super::Activate();

	if (!IsValid(WeaponMesh))
	{
		EndTask();
		return;
	}
	
	AlreadyHitActors.Reset();

	PrevStart = WeaponMesh->GetSocketLocation(StartSocket);
	PrevEnd = WeaponMesh->GetSocketLocation(EndSocket);
	bHasPrevFrame = true;
}

void UKDAbilityTask_MeleeTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!IsValid(WeaponMesh) || !GetAvatarActor())
	{
		EndTask();
		return;
	}
	TraceOnce();
}

void UKDAbilityTask_MeleeTrace::OnDestroy(bool bInOwnerFinished)
{
	AlreadyHitActors.Reset();
	Super::OnDestroy(bInOwnerFinished);
}


void UKDAbilityTask_MeleeTrace::TraceOnce()
{
	// 기능 : 한 구간 판정
	if (!IsValid(WeaponMesh)) return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* Owner = GetAvatarActor();
	if (!Owner) return;
	
	const FVector CurStart = WeaponMesh->GetSocketLocation(StartSocket);
	const FVector CurEnd = WeaponMesh->GetSocketLocation(EndSocket);

	// 안전망. Activate 시점에 메쉬가 무효였던 경우
	if (!bHasPrevFrame)
	{
		PrevStart = CurStart;
		PrevEnd = CurEnd;
		bHasPrevFrame = true;
		return;
	}
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), false, Owner);
	Params.AddIgnoredActor(Owner);
	Params.bReturnPhysicalMaterial = false;

	// 채널 응답 무시하고 Pawn ObjectType 직접 매칭
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel2); // Destructible (DefaultEngine.ini)

	switch (Mode)
	{
	case ETraceMode::Sweep : TraceSweep(ObjectParams, Params, CurStart, CurEnd); break;
	case ETraceMode::TipLine : TraceTipLine(ObjectParams, Params, CurStart, CurEnd); break;
	case ETraceMode::ArcSweep :
	case ETraceMode::ArcTri : TraceArc(ObjectParams, Params, CurStart, CurEnd); break;
	}

	// 실제 프레임 위치 - 보간점과 구분
	if (bDrawDebug)
	{
		DrawDebugPoint(World, CurStart, 14.f, FColor::Red, false, 1.0f);
		DrawDebugPoint(World, CurEnd, 14.f, FColor::Red, false, 1.0f);
	}

	PrevStart = CurStart;
	PrevEnd = CurEnd;
}

void UKDAbilityTask_MeleeTrace::TraceTipLine(const FCollisionObjectQueryParams& ObjectParams,
	const FCollisionQueryParams& Params, const FVector& CurStart, const FVector& CurEnd)
{
	// 기능 : 칼끝 이동 경로만 라인 트레이스
	UWorld* World = GetWorld();
	if (!World) return;
	
	TArray<FHitResult> Hits;
	const bool bHit = World->LineTraceMultiByObjectType(Hits, PrevEnd, CurEnd, ObjectParams, Params);
	if (bHit)
	{
		ProcessHits(Hits, Params);
	}
	if (bDrawDebug)
	{
		DrawDebugLine(World, PrevEnd, CurEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
	}
}

void UKDAbilityTask_MeleeTrace::TraceSweep(const FCollisionObjectQueryParams& ObjectParams,
	const FCollisionQueryParams& Params, const FVector& CurStart, const FVector& CurEnd)
{
	// 기능 : 무기 축 캡슐을 직선 경로로 스윕
	UWorld* World = GetWorld();
	if (!World) return;
	
	// 이동거리를 StepDist 로 나눠 서브스텝 수 산출
	const FVector PrevMid = (PrevStart + PrevEnd) * 0.5f;
	const FVector CurMid = (CurStart + CurEnd) * 0.5f;
	const float TravelDist = (CurMid - PrevMid).Size();
	const int32 SubSteps = FMath::Clamp(FMath::CeilToInt(TravelDist / StepDist), 1, MaxSubSteps);
	for (int32 Step = 0; Step < SubSteps; ++Step)
	{
		const float A0 = static_cast<float>(Step) / SubSteps;
		const float A1 = static_cast<float>(Step + 1) / SubSteps;
		
		// 칸 경계 좌표 직선 보간
		const FVector S0 = FMath::Lerp(PrevStart, CurStart, A0);
		const FVector E0 = FMath::Lerp(PrevEnd, CurEnd, A0);
		const FVector S1 = FMath::Lerp(PrevStart, CurStart, A1);
		const FVector E1 = FMath::Lerp(PrevEnd, CurEnd, A1);

		// 보간으로 만든 위치 - Step 0 의 S0·E0 는 실제 프레임이라 제외
		if (bDrawDebug && Step > 0)
		{
			DrawDebugPoint(World, S0, 7.f, FColor::Yellow, false, 1.0f);
			DrawDebugPoint(World, E0, 7.f, FColor::Yellow, false, 1.0f);
		}

		// 캡슐 자세 = 칸 끝의 무기 축
		const FVector Mid0 = (S0 + E0) * 0.5f;
		const FVector Mid1 = (S1 + E1) * 0.5f;
		const FVector Axis1 = E1 - S1;
		const float HalfH1 = FMath::Max(Axis1.Size() * 0.5f, CapsuleRadius);
		const FQuat Rot1 = FRotationMatrix::MakeFromZ(Axis1).ToQuat();
		TArray<FHitResult> Hits;
		const bool bHit = World->SweepMultiByObjectType(Hits, Mid0, Mid1, Rot1, ObjectParams,
			FCollisionShape::MakeCapsule(CapsuleRadius, HalfH1), Params);
		if (bHit)
		{
			ProcessHits(Hits, Params);
		}
		if (bDrawDebug)
		{
			DrawDebugCapsule(World, Mid1, HalfH1, CapsuleRadius, Rot1,
				bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
		}
	}
}

void UKDAbilityTask_MeleeTrace::TraceArc(const FCollisionObjectQueryParams& ObjectParams,
	const FCollisionQueryParams& Params, const FVector& CurStart, const FVector& CurEnd)
{
	// 기능 : 베지어 곡선 경로로 판정. ArcSweep = 캡슐, ArcTri = 선 격자
	UWorld* World = GetWorld();
	if (!World) return;
	
	// 제어점 = 중간점을 회전 중심 반대편으로 이동
	const FVector MidStart = (PrevStart + CurStart) * 0.5f;
	const FVector MidEnd = (PrevEnd + CurEnd) * 0.5f;
	const FVector OutDir = (MidEnd - MidStart).GetSafeNormal();
	const float StartTravel = (CurStart - PrevStart).Size();
	const float EndTravel = (CurEnd - PrevEnd).Size();
	
	// 미는 거리 = 이동거리 / StepDist. 저프레임일수록 크게 휨
	const FVector CtrlStart = MidStart + OutDir * (StartTravel / StepDist) * ArcBulge;
	const FVector CtrlEnd = MidEnd + OutDir * (EndTravel / StepDist) * ArcBulge;
	const float TravelDist = FMath::Max(StartTravel, EndTravel);
	const int32 SubSteps = FMath::Clamp(FMath::CeilToInt(TravelDist / StepDist), 1, MaxSubSteps);
	for (int32 Step = 0; Step < SubSteps; ++Step)
	{
		const float A0 = static_cast<float>(Step) / SubSteps;
		const float A1 = static_cast<float>(Step + 1) / SubSteps;
		
		// 칸 경계 좌표. 곡선 보간
		const FVector S0 = Bezier2(PrevStart, CtrlStart, CurStart, A0);
		const FVector E0 = Bezier2(PrevEnd, CtrlEnd, CurEnd, A0);
		const FVector S1 = Bezier2(PrevStart, CtrlStart, CurStart, A1);
		const FVector E1 = Bezier2(PrevEnd, CtrlEnd, CurEnd, A1);

		// 보간으로 만든 위치 - Step 0 의 S0·E0 는 실제 프레임이라 제외
		if (bDrawDebug && Step > 0)
		{
			DrawDebugPoint(World, S0, 7.f, FColor::Yellow, false, 1.0f);
			DrawDebugPoint(World, E0, 7.f, FColor::Yellow, false, 1.0f);
		}

		if (Mode == ETraceMode::ArcSweep)
		{
			const FVector Mid0 = (S0 + E0) * 0.5f;
			const FVector Mid1 = (S1 + E1) * 0.5f;
			const FVector Axis1 = E1 - S1;
			const float HalfH1 = FMath::Max(Axis1.Size() * 0.5f, CapsuleRadius);
			const FQuat Rot1 = FRotationMatrix::MakeFromZ(Axis1).ToQuat();
			
			TArray<FHitResult> Hits;
			const bool bHit = World->SweepMultiByObjectType(Hits, Mid0, Mid1, Rot1, ObjectParams,
				FCollisionShape::MakeCapsule(CapsuleRadius, HalfH1), Params);
			if (bHit)
			{
				ProcessHits(Hits, Params);
			}
			if (bDrawDebug)
			{
				DrawDebugCapsule(World, Mid1, HalfH1, CapsuleRadius, Rot1,
					bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
			}
		}
		else
		{
			// 칼 축 분할 이동선. 자루에서 칼끝까지 TraceSegments + 1 개
			for (int32 i = 0; i <= TraceSegments; ++i)
			{
				const float T = static_cast<float>(i) / TraceSegments;
				const FVector P0 = FMath::Lerp(S0, E0, T);
				const FVector P1 = FMath::Lerp(S1, E1, T);
				
				TArray<FHitResult> Hits;
				const bool bHit = World->LineTraceMultiByObjectType(Hits, P0, P1, ObjectParams, Params);
				if (bHit)
				{
					ProcessHits(Hits, Params);
				}
				if (bDrawDebug)
				{
					DrawDebugLine(World, P0, P1, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 0.5f);
				}
			}
			// 대각선. 칸마다 1개, 삼각형 2개의 경계
			for (int32 i = 0; i < TraceSegments; ++i)
			{
				const float T0 = static_cast<float>(i) / TraceSegments;
				const float T1 = static_cast<float>(i + 1) / TraceSegments;
				const FVector A = FMath::Lerp(S0, E0, T0);
				const FVector B = FMath::Lerp(S1, E1, T1);
				TArray<FHitResult> Hits;
				const bool bHit = World->LineTraceMultiByObjectType(Hits, A, B, ObjectParams, Params);
				if (bHit)
				{
					ProcessHits(Hits, Params);
				}
				if (bDrawDebug)
				{
					DrawDebugLine(World, A, B, bHit ? FColor::Magenta : FColor::Cyan, false, 1.0f, 0, 0.5f);
				}
			}
		}
	}
}

void UKDAbilityTask_MeleeTrace::ProcessHits(const TArray<FHitResult>& Hits, const FCollisionQueryParams& Params)
{
	// 기능 : 히트 필터링 후 액터당 1회 브로드캐스트
	AActor* Owner = GetAvatarActor();
	if (!Owner) return;
	
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner) continue;
		if (AlreadyHitActors.Contains(HitActor)) continue;
		if (IsWallBlocking(Hit.ImpactPoint, Params)) continue;
		
		AlreadyHitActors.Add(HitActor);
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnHit.Broadcast(Hit);
		}
	}
}

bool UKDAbilityTask_MeleeTrace::IsWallBlocking(const FVector& End, const FCollisionQueryParams& Params) const
{
	// 기능 : 정적인 메쉬가 플레이어와 히트 지점 사이를 막는지 확인
	UWorld* World = GetWorld();
	const AActor* Owner = GetAvatarActor();
	if (!World || !Owner) return false;
	
	FCollisionObjectQueryParams WallParams;
	WallParams.AddObjectTypesToQuery(ECC_WorldStatic);
	return World->LineTraceTestByObjectType(Owner->GetActorLocation(), End, WallParams, Params);
}

