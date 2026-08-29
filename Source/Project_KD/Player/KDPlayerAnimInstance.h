// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KDPlayerAnimInstance.generated.h"

class AKDPlayerCharacter;
class UCharacterMovementComponent;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
    
    // --- BlendSpace 축 입력  ---
    
    // 2D 지면 속도 크기. Peaceful/Battle 1D BS의 Speed 축
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float GroundSpeed = 0.f;
    
    // 로컬 좌우 속도. LockOn strafe BS의 LeftRight 축
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float VelocityX = 0.f;
    
    // 로컬 전후 속도. LockOn strafe BS의 FrontBack 축
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float VelocityY = 0.f;
	
	// 제자리 턴 트리거용: 컨트롤 회전 - 메시 facing의 부호있는 Yaw 차. +=우회전 필요
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float TurnYawOffset = 0.f;

	// 피벗 턴용: 메시 facing -> 이동 입력(가속도) 방향의 부호있는 Yaw 차(deg). +=우, ±180=반대
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float MovementInputAngle = 0.f;

	// 이동 입력(가속도) 유무
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHasMovementInput = false;

	// 더블점프 트리거
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bDoubleJumpTriggered = false;
    
    // --- 스테이트 전환 플래그 ) ---
    
    // 락온 중. State.Character.LockOn loose 태그. Free<->LockOn 전환
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsLockedOn = false;

	// 조준 여부 - AimOffset 전환
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsAiming = false;
	
	// 가드 여부 - 가드 로코 전환
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsBlocking = false;

	// 조준 상하  아래 -1 | 정면 0 | 위 +1  조준 X 0
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float AimPitch = 0.f;

	// 조준 좌우  왼쪽 -1 | 정면 0 | 오른쪽 +1  조준 X 0
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float AimYaw = 0.f;

	// AimYaw 최대가 되는 좌우 각도
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "10.0", ClampMax = "180.0"))
	float AimYawRange = 60.f;
	
    // 주변 적 존재(InCombat 태그). Peaceful<->Battle 전환
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInBattleStance = false;
    
    // 공중(점프/낙하). Ground<->Airborne 전환
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInAir = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsSprinting = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsFullSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDoubleJumping = false;

private:
	// 캐시된 소유 객체 (게임스레드 접근).
	UPROPERTY(Transient)
	TObjectPtr<AKDPlayerCharacter> OwningPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovementComp;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> ASC;

	// 게임스레드 -> 워커스레드 스냅샷 
	FVector CachedVelocity = FVector::ZeroVector;
	FRotator CachedActorRotation = FRotator::ZeroRotator;
	FVector CachedAcceleration = FVector::ZeroVector;
	
	int32 CachedJumpCount = 0;
	int32 PrevJumpCount = 0;
	
	float CachedControlYaw = 0.f;
	float CachedRailAlpha = 0.f;   // 마우스 상하 각도 0~1  게임스레드 스냅샷
	float CachedAimPitchScale = 0.f;
	
};
