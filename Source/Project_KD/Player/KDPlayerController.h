#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KDPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS()
class PROJECT_KD_API AKDPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> IA_Jump;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
    TObjectPtr<UInputAction> IA_Walk;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_LightAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_HeavyAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_Parry;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_LockOnToggle;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	TObjectPtr<UInputAction> IA_Execute;
private:
	void Handle_Move(const FInputActionValue& Value);
	void Handle_Look(const FInputActionValue& Value);
	void Handle_Jump();
	void Handle_StopJump();
	void Handle_SprintStart();
	void Handle_SprintStop();
	void Handle_WalkToggle();
	void Handle_LightAttack();
	void Handle_HeavyAttack();
	void Handle_Dodge();
	void Handle_Parry();
	void Handle_ParryStop();
	void Handle_LockOnToggle();
	void Handle_Execute();
};
