// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/KDLockOnComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/KDLockOnConfig.h"
#include "Interface/KDTargetableInterface.h"
#include "KDGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Curves/CurveFloat.h"

// Sets default values for this component's properties
UKDLockOnComponent::UKDLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 위젯 컴포넌트 셋팅
	ReticleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ReticleWidget"));
	ReticleWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ReticleWidgetComponent->SetVisibility(false);
	ReticleWidgetComponent->SetDrawAtDesiredSize(true);
}


void UKDLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// 위젯 적용
	if (Config && Config->ReticleWidgetClass)
	{
		ReticleWidgetComponent->SetWidgetClass(Config->ReticleWidgetClass);
	}

	RegisterAimingTagListener();
}

void UKDLockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AimingTagHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_Aiming, EGameplayTagEventType::NewOrRemoved).Remove(AimingTagHandle);
		}
		AimingTagHandle.Reset();
	}
	
	Super::EndPlay(EndPlayReason);
}

void UKDLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 락온 안 한 상태엔 끄기
	if (!bIsLockedOn) return;

	// 자동 해제 + 자동 전환 
	if (!IsTargetStillValid())
	{
		DisengageLockOn();
		if (AActor* NewTarget = FindBestTarget())
		{
			EngageLockOn(NewTarget);
		}
	}

	// 락온 카메라 추적 — Yaw 보간 후 PC ControlRotation 설정. SpringArm/캐릭터 회전 자동 따라옴.
	if (Config)
	{
		if (AActor* Target = LockedTarget.Get())
		{
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
			if (PC)
			{
				const FVector TargetPoint = IKDTargetableInterface::Execute_GetLockOnPoint(Target);
				const FVector ToTarget = TargetPoint - PC->PlayerCameraManager->GetCameraLocation(); // 카메라에서 적 방향 벡터
				const FRotator CurrentRot = PC->GetControlRotation();
				const FRotator LookRot = ToTarget.Rotation(); // 방향벡터를 각도
				
				// 각도가 클수록 Yaw 회전 속도 증가
				float YawSpeed = Config->CameraInterpSpeed;
				if (Config->YawSpeedByAngle)
				{
					const float YawDiff = FMath::Abs(FRotator::NormalizeAxis(LookRot.Yaw - CurrentRot.Yaw));
					YawSpeed = Config->YawSpeedByAngle->GetFloatValue(YawDiff);
				}
				
				FRotator InterpedRot = CurrentRot;
				if (YawSpeed > 0.f)   // 0을 넘기면 가만히가 아니라 즉시 스냅 - 생략
				{
					const FRotator DesiredRot(CurrentRot.Pitch, LookRot.Yaw, CurrentRot.Roll); // 상하 유지, 좌우만 적 쪽
					InterpedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, YawSpeed);
				}

				if (Config->LockOnPitchCurve)
				{
					const float Dist = FVector::Dist(OwnerPawn->GetActorLocation(), TargetPoint);
					const float CurrentPitch = FRotator::NormalizeAxis(CurrentRot.Pitch); // 0~360으로 오는 값을 -180~180
					InterpedRot.Pitch =
						FMath::FInterpTo(CurrentPitch,
							Config->LockOnPitchCurve->GetFloatValue(Dist),
							DeltaTime,
							Config->PitchInterpSpeed);
				}
				
				PC->SetControlRotation(InterpedRot);
			}
		}
	}
}

void UKDLockOnComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		DisengageLockOn();
		return;
	}

	if (AActor* Target = FindBestTarget())
	{
		EngageLockOn(Target);
	}
}

AActor* UKDLockOnComponent::FindBestTarget() const
{
	// 기능 : 락온 대상 검색
	return Config ? FindTargetByFilter(Config->TargetFilter) : nullptr;
}

AActor* UKDLockOnComponent::FindTargetByFilter(const FKDTargetFilter& Filter) const
{
	// 기능 : 필터 조건으로 대상 1명 선택
	TArray<AActor*> Candidates;
	GatherCandidates(Filter, Candidates);
	if (Candidates.Num() == 0) return nullptr;
	
	const FVector OwnerLoc = GetOwner()->GetActorLocation();
	const FVector Basis = GetFilterBasis(Filter);
	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Lowest();
	for (AActor* Candidate : Candidates)
	{
		const FVector CandLoc = Candidate->GetActorLocation();
		
		// 각도순 = 내적 최대 | 최단거리 = 거리 부호 반전 후 최대
		float Score;
		if (Filter.SortType == EKDTargetSortType::SmallestAngle)
		{
			Score = FVector::DotProduct(Basis, (CandLoc - OwnerLoc).GetSafeNormal2D());
		}
		else
		{
			Score = -FVector::DistSquared2D(OwnerLoc, CandLoc);
		}

		// 점수가 최고점수보다 높으면 교체
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Candidate;
		}
	}
#if !UE_BUILD_SHIPPING
	if (Filter.bDrawDebug) DrawFilterDebug(Filter, Basis, Candidates, Best);
#endif
	return Best;
}

void UKDLockOnComponent::EngageLockOn(AActor* NewTarget)
{
	if (!NewTarget) return;

	bIsLockedOn = true;
	LockedTarget = NewTarget;

	// GAS 태그 — KDPlayerCameraManager(다음 Step) + ABP가 락온 상태 인지.
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->AddLooseGameplayTag(GameplayTags::State_Character_LockOn);
	}

	// 인터페이스 알림 — 적 ABP에서 LockOn_BS 활성화 등.
	if (NewTarget->Implements<UKDTargetableInterface>())
	{
		IKDTargetableInterface::Execute_OnTargeted(NewTarget, true);
	}

	// 마커 위젯 Target 위로 — Screen mode라 캐릭터 머리 살짝 위 자연스럽게 표시.
	if (ReticleWidgetComponent && NewTarget->GetRootComponent())
	{
		ReticleWidgetComponent->AttachToComponent(
			NewTarget->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetIncludingScale);

		// Enemy LockOnPoint Get
		const FVector TargetPoint = IKDTargetableInterface::Execute_GetLockOnPoint(NewTarget);
		const float RelativeZ = (TargetPoint - NewTarget->GetActorLocation()).Z;
		ReticleWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, RelativeZ ));  // 머리 위 오프셋
		ReticleWidgetComponent->SetVisibility(true);
	}
}

void UKDLockOnComponent::DisengageLockOn()
{
	if (!bIsLockedOn) return;

	// 인터페이스 알림 — 적 ABP LockOn_BS 비활성화.
	if (AActor* PrevTarget = LockedTarget.Get())
	{
		if (PrevTarget->Implements<UKDTargetableInterface>())
		{
			IKDTargetableInterface::Execute_OnTargeted(PrevTarget, false);
		}
	}

	// GAS 태그 제거.
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->RemoveLooseGameplayTag(GameplayTags::State_Character_LockOn);
	}

	// 마커 위젯 — Owner로 복귀 + 숨김.
	if (ReticleWidgetComponent && GetOwner() && GetOwner()->GetRootComponent())
	{
		ReticleWidgetComponent->SetVisibility(false);
		ReticleWidgetComponent->AttachToComponent(
			GetOwner()->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	bIsLockedOn = false;
	LockedTarget = nullptr;
}


bool UKDLockOnComponent::IsTargetStillValid() const
{
	if (!Config) return false;
	AActor* Target = LockedTarget.Get();
	if (!IsValid(Target)) return false;

	// 사망/무적 등 → CanBeTargeted false
	if (Target->Implements<UKDTargetableInterface>())
	{
		if (!IKDTargetableInterface::Execute_CanBeTargeted(Target)) return false;
	}

	// 거리 초과 -> 해제 
	APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner) return false;
	const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());

	// 필터 반지름보다 높으면 해제
	if (DistSq > FMath::Square(Config->TargetFilter.Radius)) return false;

	// LoS 잃음 → 해제 (장애물 뒤)
	if (!HasLineOfSightTo(Target)) return false;

	return true;
}

void UKDLockOnComponent::GatherCandidates(const FKDTargetFilter& Filter, TArray<AActor*>& OutCandidates) const
{
	// 기능 : 필터 범위 안의 자격 있는 후보 수집
	OutCandidates.Reset();
	const APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner) return;
	
	const FVector OwnerLoc = Owner->GetActorLocation();
	// 세로 중심 = 발밑 오프셋 + 높이 절반
	const float HalfHeight = Filter.Height * 0.5f;
	const FVector QueryCenter = OwnerLoc + FVector(0.f, 0.f, Filter.HeightOffset + HalfHeight);
	
	// 박스로 넓게 줍고 아래에서 가로 거리로 원기둥 생성 
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		QueryCenter,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeBox(FVector(Filter.Radius, Filter.Radius, HalfHeight)),
		Params);
	
	// 부채꼴만 반각 적용 — 원기둥은 전방위
	const bool bUseAngle = (Filter.ShapeType == EKDTargetShapeType::Arc);
	const float CosThreshold = bUseAngle ? FMath::Cos(FMath::DegreesToRadians(Filter.HalfAngle)) : -1.f;
	const FVector Basis = GetFilterBasis(Filter);
	const float RadiusSq = FMath::Square(Filter.Radius);
	TSet<AActor*> Seen;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		// null / 자기 자신 / 중복 액터(여러 콜리전 보유) 제거
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Owner || Seen.Contains(Candidate)) continue;
		Seen.Add(Candidate);
		
		// 박스 모서리 제거 — 가로 거리로 원기둥 완성
		const FVector CandLoc = Candidate->GetActorLocation();
		if (FVector::DistSquared2D(OwnerLoc, CandLoc) > RadiusSq) continue;
		
		// 락온 받을 자격 + 락온 가능한 상태
		if (!Candidate->Implements<UKDTargetableInterface>()) continue;
		if (!IKDTargetableInterface::Execute_CanBeTargeted(Candidate)) continue;
		
		// 부채꼴 반각 — 기준 벡터와 (나 -> 후보) 방향의 내적
		if (bUseAngle)
		{
			const FVector ToCand = (CandLoc - OwnerLoc).GetSafeNormal2D();
			if (FVector::DotProduct(Basis, ToCand) < CosThreshold) continue;
		}
		
		// 장애물 뒤 적 제외
		if (!HasLineOfSightTo(Candidate)) continue;
		OutCandidates.Add(Candidate);
	}
}

FVector UKDLockOnComponent::GetFilterBasis(const FKDTargetFilter& Filter) const
{
	// 기능 : 반각 기준 벡터
	const APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner) return FVector::ForwardVector;

	// 플레이어 카메라 기준
	if (Filter.Basis == EKDTargetBasisType::Camera)
	{
		if (const APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
		{
			FVector CamLoc;
			FRotator CamRot;
			PC->GetPlayerViewPoint(CamLoc, CamRot);
			return CamRot.Vector().GetSafeNormal2D();
		}
	}

	// 캐릭터 정면
	return Owner->GetActorForwardVector().GetSafeNormal2D();
}

#if !UE_BUILD_SHIPPING
void UKDLockOnComponent::DrawFilterDebug(const FKDTargetFilter& Filter, const FVector& Basis,
	const TArray<AActor*>& Candidates, const AActor* Chosen) const
{
	// 기능 : 필터 범위  후보  선택 대상 표시
	const UWorld* World = GetWorld();
	const AActor* Owner = GetOwner();
	if (!World || !Owner) return;
	constexpr float Duration = 1.0f;
	const FVector Foot = Owner->GetActorLocation() + FVector(0.f, 0.f, Filter.HeightOffset);
	const FVector Top = Foot + FVector(0.f, 0.f, Filter.Height);
	
	// 원기둥 = 전방위 통짜 | 부채꼴 = 위아래 호 + 경계선
	if (Filter.ShapeType == EKDTargetShapeType::Cylinder)
	{
		DrawDebugCylinder(World, Foot, Top, Filter.Radius, 24, FColor::Cyan, false, Duration);
	}
	else
	{
		constexpr int32 Segments = 16;
		const float Step = (Filter.HalfAngle * 2.f) / Segments;
		for (const FVector& Level : { Foot, Top })
		{
			// 호 — 세그먼트를 이어 그린다
			FVector Prev = Level + Basis.RotateAngleAxis(-Filter.HalfAngle, FVector::UpVector) * Filter.Radius;
			for (int32 i = 1; i <= Segments; ++i)
			{
				const FVector Cur = Level + Basis.RotateAngleAxis(-Filter.HalfAngle + Step * i, FVector::UpVector) * Filter.Radius;
				DrawDebugLine(World, Prev, Cur, FColor::Cyan, false, Duration, 0, 2.f);
				Prev = Cur;
			}
			// 좌우 경계선
			DrawDebugLine(World, Level,
				Level + Basis.RotateAngleAxis(-Filter.HalfAngle, FVector::UpVector) * Filter.Radius,
				FColor::Cyan, false, Duration, 0, 2.f);
			DrawDebugLine(World, Level,
				Level + Basis.RotateAngleAxis(Filter.HalfAngle, FVector::UpVector) * Filter.Radius,
				FColor::Cyan, false, Duration, 0, 2.f);
		}
	}
	
	// 기준 벡터
	DrawDebugLine(World, Foot, Foot + Basis * Filter.Radius, FColor::White, false, Duration, 0, 3.f);
	// 통과 후보 초록 | 최종 선택 빨강
	for (const AActor* Candidate : Candidates)
	{
		const bool bChosen = (Candidate == Chosen);
		DrawDebugSphere(World, Candidate->GetActorLocation(), bChosen ? 60.f : 40.f, 12,
			bChosen ? FColor::Red : FColor::Green, false, Duration);
	}
}
#endif

bool UKDLockOnComponent::HasLineOfSightTo(const AActor* Target) const
{
	// 기능 : 나 -> 대상 직선을 월드 지오메트리가 막는지 — 후보 검색과 락온 유지 공용
	if (!Config || !Config->bUseLineOfSightCheck) { return true; }

	const AActor* Owner = GetOwner();
	if (!Owner || !IsValid(Target)) { return false; }

	// 월드 지오메트리만 조회 — ECC_Visibility 채널은 Pawn 이 무시해 적이 적을 가리지 X
	// 채널 대신 오브젝트 타입을 쓰는 이유 = AT_MeleeTrace.cpp:89 와 동일
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.AddIgnoredActor(Target);

	return !GetWorld()->LineTraceTestByObjectType(
		Owner->GetActorLocation(), Target->GetActorLocation(), ObjParams, Params);
}

void UKDLockOnComponent::RegisterAimingTagListener()
{
	// 기능 : 조준 태그 변화 확인 등록
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		// ASC가 준비 X -> 재시도
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UKDLockOnComponent::RegisterAimingTagListener);
		}
		return;
	}

	// 태그 구독
	AimingTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_Aiming, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UKDLockOnComponent::OnAimingTagChanged);
}

void UKDLockOnComponent::OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0 && bIsLockedOn)
	{
		DisengageLockOn();
	}
}

