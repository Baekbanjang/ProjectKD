// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/KDPlayerAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "KDPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UKDPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 이동 컴포넌트 캐싱
	OwningPlayer = Cast<AKDPlayerCharacter>(TryGetPawnOwner());
	if (OwningPlayer)
	{
		MovementComp = OwningPlayer->GetCharacterMovement();
	}
}

void UKDPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Pawn이 init 시점에 없었으면 재시도.
	if (!OwningPlayer)
	{
		OwningPlayer = Cast<AKDPlayerCharacter>(TryGetPawnOwner());
		if (!OwningPlayer)
		{
			return;
		}
		MovementComp = OwningPlayer->GetCharacterMovement();
	}

	// 워커스레드가 쓸 속도/회전 스냅샷.
	CachedVelocity = OwningPlayer->GetVelocity();
	CachedActorRotation = OwningPlayer->GetActorRotation();
	CachedVelocity = OwningPlayer->GetVelocity();
	CachedActorRotation = OwningPlayer->GetActorRotation();
	CachedControlYaw = OwningPlayer->GetControlRotation().Yaw;
	CachedRailAlpha = OwningPlayer->GetCameraRailAlpha();
	CachedAcceleration = MovementComp ? MovementComp->GetCurrentAcceleration() : FVector::ZeroVector;
	CachedJumpCount = OwningPlayer ? OwningPlayer->JumpCurrentCount : 0;

	bIsInAir = MovementComp ? MovementComp->IsFalling() : false;

	bIsSprinting = OwningPlayer->IsSprinting();
	bIsFullSprinting = OwningPlayer->IsFullSprinting();

	// ASC 없으면 획득 후 태그 쿼리.
	if (!ASC)
	{
		ASC = OwningPlayer->GetAbilitySystemComponent();
	}
	if (ASC)
	{
		bIsLockedOn = ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn);

		bIsInBattleStance = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InCombat);

		bIsAiming = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming);
	}
}

void UKDPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	GroundSpeed = CachedVelocity.Size2D();

	// 월드 속도를 캐릭터 로컬로 변환: X=전후, Y=좌우.
	const FVector LocalVelocity = CachedActorRotation.UnrotateVector(CachedVelocity);
	VelocityY = LocalVelocity.X; // 전후 (FrontBack)
	VelocityX = LocalVelocity.Y; // 좌우 (LeftRight)

	TurnYawOffset = FMath::FindDeltaAngleDegrees(CachedActorRotation.Yaw, CachedControlYaw);
	
	// AimOffset 축 값  조준 X 0 = 중앙 = 상체 유지
	AimPitch = bIsAiming ? (CachedRailAlpha * 2.f - 1.f) : 0.f;
	AimYaw   = bIsAiming ? FMath::Clamp(TurnYawOffset / AimYawRange, -1.f, 1.f) : 0.f;

	const FVector AccelDir = CachedAcceleration.GetSafeNormal2D();
	MovementInputAngle = AccelDir.IsNearlyZero()
		? 0.f
		: FMath::FindDeltaAngleDegrees(CachedActorRotation.Yaw, AccelDir.Rotation().Yaw);

	// 키를 누르고 있는지 확인
	bHasMovementInput = !AccelDir.IsNearlyZero(); // IsNearlyZero -> 입력 없으면 True

	bDoubleJumpTriggered = (CachedJumpCount >= 2) && (CachedJumpCount > PrevJumpCount);
	PrevJumpCount = CachedJumpCount;

	bIsDoubleJumping = (CachedJumpCount >= 2) && bIsInAir;
}
