// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/KDSpringArmComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Components/SplineComponent.h"

UKDSpringArmComponent::UKDSpringArmComponent()
{
	bUsePawnControlRotation = true; // 마우스 회전 따라감
	bInheritPitch = false;  // 피치(상하)는 제외
}

void UKDSpringArmComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	UpdateAimAlpha(DeltaTime);
	ApplyRailPosition();   // 레일에서 프레임마다 위치 확보
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);   // 랙 | 충돌 | 소켓 갱신
	UpdateLookRotation(); // 회전 업데이트
	UpdateChildTransforms();   // 카메라에 회전 반영 - GetSocketTransform(KD) 호출
}

FTransform UKDSpringArmComponent::GetSocketTransform(FName InSocketName,
	ERelativeTransformSpace TransformSpace) const
{
	if (!DollySpline)
	{
		return Super::GetSocketTransform(InSocketName, TransformSpace);   // 레일 없음 - 기본 스프링암 동작
	}

	// 카메라 위치 + 카메라 방향 묶음
	// 위치 = 부모가 랙 | 충돌까지 반영해 계산한 값
	// 방향 = UpdateLookRotation이 구해둔 캐릭터 조준값
	// 부모 기본 방향 = 스프링암이 뻗은 방향이라 캐릭터 조준 X 
	const FTransform RelativeTransform(RelativeLookRotation, RelativeSocketLocation);
	switch (TransformSpace)
	{
		case RTS_World: // 월드 좌표
			return RelativeTransform * GetComponentTransform();

		case RTS_Actor: // 캐릭터 기준
			if (const AActor* OwnerActor = GetOwner())
			{
				return (RelativeTransform * GetComponentTransform()).GetRelativeTransform(OwnerActor->GetTransform());
			}
			break;

		case RTS_Component: // 카메라 스프링
			return RelativeTransform;

		default: ;
	}

	return RelativeTransform;
}

void UKDSpringArmComponent::UpdateAimAlpha(float DeltaTime)
{
	// 기능 : 조준 태그를 보고 두 스플라인 섞는 비율을 부드럽게 변경
	if (!CachedASC.IsValid())
	{
		CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}
	// 조준 여부
	const bool bAiming = CachedASC.IsValid()
		&& CachedASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming);
	AimAlpha = FMath::FInterpTo(AimAlpha, bAiming ? 1.f : 0.f, DeltaTime, AimBlendSpeed);
}

FVector UKDSpringArmComponent::SampleRail(const USplineComponent* Rail, float Alpha) const
{
	// 기능 : 진행도를 레일 번호로 바꿔서 그 자리 좌표를 추출
	const float LastKey = static_cast<float>(Rail->GetNumberOfSplinePoints() - 1); // 마지막 포인트 추출
	return Rail->GetLocationAtSplineInputKey(Alpha * LastKey, ESplineCoordinateSpace::Local); // 3d 좌표 추출
}

void UKDSpringArmComponent::ApplyRailPosition()
{
	// 기능 : 마우스의 각도를 보고 카메라스프링의 길이와 오프셋을 변경
	if (!DollySpline)
	{
		return;   // 레일 미지정
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());

	const AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (!OwnerController)
	{
		return;   // 조종자 없음 - 마우스 각도 출처 X
	}

	// 마우스 상하 각도
	const float Pitch = FRotator::NormalizeAxis(OwnerController->GetControlRotation().Pitch);

	// 각도 범위 0~1 범위로 변경
	const float Alpha = FMath::Clamp((Pitch - PitchAtStart) / (PitchAtEnd - PitchAtStart), 0.f, 1.f);
	
	FVector Point = SampleRail(DollySpline, Alpha);

	// 조준 중이면 조준 스플라인과 섞음
	if (AimDollySpline && AimAlpha > KINDA_SMALL_NUMBER)
	{
		Point = FMath::Lerp(Point, SampleRail(AimDollySpline, Alpha), AimAlpha);
	}

	TargetArmLength = -Point.X;                          // 카메라 걸이
	SocketOffset = FVector(0.f, Point.Y, Point.Z);   // 카메라 오프셋
}

void UKDSpringArmComponent::UpdateLookRotation()
{
	// 기능 : 랙 | 충돌 전의 이상 위치에서 캐릭터를 보는 각도를 구해서 저장
	// 랙 먹은 실제 위치로 각도를 내면 뒤처짐이 회전으로 새어 화면 전체가 스윙
	if (!DollySpline)
	{
		return;
	}
	
	// 스프링암 로컬 기준 - 랙 | 충돌이 없을 때의 카메라 자리
	const FVector IdealCam(-TargetArmLength, SocketOffset.Y, SocketOffset.Z);
	
	// 조준점도 같은 어깨 오프셋(SocketOffset.Y)만큼 옆으로 - 좌우 성분 상쇄로 요 계산 소멸
	const FVector Aim(0.f, SocketOffset.Y, LookAtHeightOffset); //카메라가 겨누는 표적 = 캐릭터 몸통.
	const FVector Dir = Aim - IdealCam;   // Y 성분 0 - 순수 상하 기울기
	
	// 요는 마우스에서 직접 - 랙을 거치지 않아 흔들리지 않음
	const FRotator ArmRot = GetTargetRotation();
	
	// 피치 = 화살표 상하 기울기 | 요 = 마우스 직수령 | 롤 = 0 고정
	const FRotator WorldLook(FMath::RadiansToDegrees(FMath::Atan2(Dir.Z, Dir.X)), ArmRot.Yaw, 0.f);
	
	// GetSocketTransform이 GetComponentTransform을 다시 곱해 월드로 복원
	RelativeLookRotation = GetComponentQuat().Inverse() * WorldLook.Quaternion();
}
