// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/BaseCharacter.h"
#include "KDPlayerCharacter.generated.h"

class UHitStopComponent;
class USprintComponent;
class UMotionWarpingComponent;
class UCombatStateComponent;
class ULockOnComponent;
struct FGameplayTag;
class UWeaponComponent;
class UCameraComponent;
class UInputBufferComponent;
class UKDSpringArmComponent;
class USplineComponent;
class UComboComponent;

UCLASS()
class PROJECT_KD_API AKDPlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AKDPlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	// Wire to input in BP. Activates any granted ability tagged Ability.Mugong.Light.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryLightAttack() const;

	// Wire to input in BP. Activates any granted ability tagged Ability.Mugong.Dodge.
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
	
	// 붐 끝단 카메라. 자체 회전 없음(붐이 회전 담당).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// 선입력 버퍼 (D6). BP -> TryLightAttack -> Push, GA→OnComboWindowOpen -> TryConsume.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputBufferComponent> InputBuffer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponComponent> WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponComponent> GunWeaponComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UComboComponent> ComboComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<ULockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatStateComponent> CombatStateComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHitStopComponent> HitStopComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	TObjectPtr<USprintComponent> SprintComp;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Turn")
	TObjectPtr<UCurveFloat> TurnSpeedCurve;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion")
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// 처형 발동 거리
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "50", ClampMax = "1000"))
	float ExecutionRange = 250.f;

public:
	UComboComponent* GetComboComponent() const {return ComboComp;}
	ULockOnComponent* GetLockOnComponent() const {return LockOnComponent;}

	bool IsSprinting() const;     
	bool IsFullSprinting() const; 
	bool IsWalking() const;       

private:
	void TryConsumeAndActivate(UAbilitySystemComponent* ASC, bool bCanCancel, const FGameplayTag& InputTag, const FGameplayTag& AbilityTag);
	
	UFUNCTION()
	void ApplyMaxWalkSpeed(float NewSpeed);

	bool ActivateByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
	void CancelByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
};
