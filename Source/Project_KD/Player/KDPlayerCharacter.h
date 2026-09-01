// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/KDBaseCharacter.h"
#include "KDPlayerCharacter.generated.h"

class UKDHitStopComponent;
class UKDSprintComponent;
class UMotionWarpingComponent;
class UKDCombatStateComponent;
class UKDLockOnComponent;
struct FGameplayTag;
class UKDWeaponComponent;
class UCameraComponent;
class UKDInputBufferComponent;
class UKDSpringArmComponent;
class USplineComponent;
class UKDComboComponent;
class UKDPlayerAbilityInputComponent;

UCLASS()
class PROJECT_KD_API AKDPlayerCharacter : public AKDBaseCharacter
{
	GENERATED_BODY()

public:
	AKDPlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	// Wire to input in BP. Activates any granted ability tagged Ability.Player.Light.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryLightAttack() const;

	// Wire to input in BP. Activates any granted ability tagged Ability.Player.Dodge.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryDodge() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryHeavyAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryParry() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryParryStop() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryExecute() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryAimStart() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryAimStop() const;

	// 숫자키 스킬 발동 — 1~4
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TrySkill(int32 SkillIndex) const;

	// 차지 스킬 키 릴리즈 통지
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TrySkillHoldStop() const;

	void StartSprint();
	void StopSprint();

	void ToggleWalk();  

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();
	
protected:
	// 3인칭 백뷰 붐. 캡슐에 부착, 컨트롤러 회전을 따라 캐릭터 뒤를 공전. 길이/오프셋은 BP에서 튜닝.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UKDSpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USplineComponent> CameraDollySpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USplineComponent> AimDollySpline;
	
	// 붐 끝단 카메라. 자체 회전 없음(붐이 회전 담당).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// 선입력 버퍼 (D6). BP -> TryLightAttack -> Push, GA→OnComboWindowOpen -> TryConsume.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UKDInputBufferComponent> InputBuffer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UKDWeaponComponent> WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UKDWeaponComponent> GunWeaponComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDComboComponent> ComboComp;

	// 입력 -> 어빌리티 활성화 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDPlayerAbilityInputComponent> AbilityInputComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<UKDLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDCombatStateComponent> CombatStateComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UKDHitStopComponent> HitStopComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	TObjectPtr<UKDSprintComponent> SprintComp;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Turn")
	TObjectPtr<UCurveFloat> TurnSpeedCurve;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion")
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// 락온, 가드 중 최고 이동속도
	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "50.0", ClampMax = "800.0"))
	float LockOnMoveSpeed = 280.f;

	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = "50.0", ClampMax = "800.0"))
	float AimMoveSpeed = 167.f;

	UPROPERTY(EditAnywhere, Category = "Block", meta = (ClampMin = "50.0", ClampMax = "800.0"))
	float BlockMoveSpeed = 167.f;

	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float AimPitchScale = 0.33f;
	
public:
	UKDComboComponent* GetComboComponent() const {return ComboComp;}
	UKDLockOnComponent* GetLockOnComponent() const {return LockOnComponent;}
	UKDPlayerAbilityInputComponent* GetAbilityInputComponent() const {return AbilityInputComp;}
	
	// 마우스 상하 각도 0~1  애님 조준용
	float GetCameraRailAlpha() const;
	float GetAimPitchScale() const { return AimPitchScale; }

	bool IsSprinting() const;
	bool IsFullSprinting() const;

private:
	UFUNCTION()
	void ApplyMaxWalkSpeed(float NewSpeed);

	void RefreshMaxWalkSpeed();
	float BaseWalkSpeed = 500.f;
};
