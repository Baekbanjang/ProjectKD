// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LockOnComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/LockOnConfig.h"
#include "Interface/KDTargetableInterface.h"
#include "KDGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Curves/CurveFloat.h"

// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 위젯 컴포넌트 셋팅
	ReticleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ReticleWidget"));
	ReticleWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ReticleWidgetComponent->SetVisibility(false);
	ReticleWidgetComponent->SetDrawAtDesiredSize(true);
}


void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	// 위젯 적용
	if (Config && Config->ReticleWidgetClass)
	{
		ReticleWidgetComponent->SetWidgetClass(Config->ReticleWidgetClass);
	}

	RegisterAimingTagListener();
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

void ULockOnComponent::ToggleLockOn()
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

AActor* ULockOnComponent::FindBestTarget(float OverrideRadius, float OverrideConeAngle) const
{
	if (!Config) return nullptr;
	APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner) return nullptr;
	APlayerController* PC = Cast<APlayerController>(Owner->GetController());
	if (!PC) return nullptr;

	const FVector OwnerLoc = Owner->GetActorLocation();

	// 인자 없으면 기본값 -1, 있으면  Config(>0.f는 기본값 -1.f라서)
	const float SearchRadius = (OverrideRadius > 0.f) ? OverrideRadius : Config->LockOnRadius;
	const float SearchConeAngle = (OverrideConeAngle > 0.f) ? OverrideConeAngle : Config->ViewConeAngle;

	// 카메라 forward — 시야 콘 기준점
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector CamForward = CamRot.Vector();

	// SearchRadius 반경 Pawn 후보 모두 수집
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OwnerLoc,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(SearchRadius),
		Params);
	
	// 시야 콘 임계값 — ViewConeAngle=90이면 ±45도 → cos(45)≈0.707.
	const float HalfAngleRad = FMath::DegreesToRadians(SearchConeAngle * 0.5f);
	const float CosThreshold = FMath::Cos(HalfAngleRad);

	AActor* BestTarget = nullptr;
	float BestDot = -1.f; // 각도 최소 = 내적 최대 (마주보면 +1)
	TSet<AActor*> Seen;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		// null / 자기 자신 / 중복 액터(여러 콜리전 보유) 제거.
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Owner || Seen.Contains(Candidate)) continue;
		Seen.Add(Candidate);

		// IKDTargetable 인터페이스 + CanBeTargeted — Cast 회피, 적이 락온 자격 자가 판단.
		if (!Candidate->Implements<UKDTargetableInterface>()) continue; // 락온 받을 자격 있는지
		if (!IKDTargetableInterface::Execute_CanBeTargeted(Candidate)) continue; // 락온 가능한 상태인지

		// 시야 콘 — 카메라 forward와 (Owner→Candidate) 방향 Dot product.
		// 내적을 통해 시야 방향인지 확인
		const FVector ToCand = (Candidate->GetActorLocation() - OwnerLoc).GetSafeNormal();
		const float Dot = FVector::DotProduct(CamForward, ToCand);
		if (Dot < CosThreshold) continue;

		// LoS — 나 -> 적 직선에 벽이 있으면 제외 (장애물 뒤 적 불가)
		if (!HasLineOfSightTo(Candidate)) continue;

		// 콘 중심선에 가장 가까운 적 우선 — 내적이 클수록 각도가 작음
		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestTarget = Candidate;
		}
	}
	
	return BestTarget;
}

void ULockOnComponent::EngageLockOn(AActor* NewTarget)
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

void ULockOnComponent::DisengageLockOn()
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


bool ULockOnComponent::IsTargetStillValid() const
{
	if (!Config) return false;
	AActor* Target = LockedTarget.Get();
	if (!IsValid(Target)) return false;

	// 사망/무적 등 → CanBeTargeted false.
	if (Target->Implements<UKDTargetableInterface>())
	{
		if (!IKDTargetableInterface::Execute_CanBeTargeted(Target)) return false;
	}

	// 거리 초과 → 해제 
	APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner) return false;
	const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());
	if (DistSq > Config->LockOnRadius * Config->LockOnRadius) return false;

	// LoS 잃음 → 해제 (장애물 뒤).
	if (!HasLineOfSightTo(Target)) return false;

	return true;
}

bool ULockOnComponent::HasLineOfSightTo(const AActor* Target) const
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

void ULockOnComponent::RegisterAimingTagListener()
{
	// 기능 : 조준 태그 변화 확인 등록
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		// ASC가 준비 X -> 재시도
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &ULockOnComponent::RegisterAimingTagListener);
		}
		return;
	}

	// 태그 구독
	AimingTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_Aiming, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ULockOnComponent::OnAimingTagChanged);
}

void ULockOnComponent::OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0 && bIsLockedOn)
	{
		DisengageLockOn();
	}
}

