#include "Player/KDPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Player/KDPlayerCharacter.h"

void AKDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AKDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent);

	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AKDPlayerController::Handle_Move);
		// 패드: 스틱 중립(Completed) 되면 달리기 자동 해제 — 키보드 SprintStop(Shift 뗌)과 동일 경로
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AKDPlayerController::Handle_SprintStop);
	}
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AKDPlayerController::Handle_Look);
	}
	if (IA_Jump)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AKDPlayerController::Handle_Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AKDPlayerController::Handle_StopJump);
	}
	if (IA_Sprint)
	{
		EIC->BindAction(IA_Sprint, ETriggerEvent::Triggered, this, &AKDPlayerController::Handle_SprintStart);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AKDPlayerController::Handle_SprintStop);
	}
	if (IA_Walk)
	{
		EIC->BindAction(IA_Walk, ETriggerEvent::Started, this, &AKDPlayerController::Handle_WalkToggle);
	}
	if (IA_LightAttack)
	{
		EIC->BindAction(IA_LightAttack, ETriggerEvent::Started, this, &AKDPlayerController::Handle_LightAttack);
	}
	if (IA_HeavyAttack)
	{
		EIC->BindAction(IA_HeavyAttack, ETriggerEvent::Started, this, &AKDPlayerController::Handle_HeavyAttack);
	}
	if (IA_Dodge)
	{
		EIC->BindAction(IA_Dodge, ETriggerEvent::Triggered, this, &AKDPlayerController::Handle_Dodge);
	}
	if (IA_Parry)
	{
		EIC->BindAction(IA_Parry, ETriggerEvent::Started, this, &AKDPlayerController::Handle_Parry);
		EIC->BindAction(IA_Parry, ETriggerEvent::Completed, this, &AKDPlayerController::Handle_ParryStop);
	}
	if (IA_LockOnToggle)
	{
		EIC->BindAction(IA_LockOnToggle, ETriggerEvent::Started, this, &AKDPlayerController::Handle_LockOnToggle);
	}
	if (IA_Execute)
	{
		EIC->BindAction(IA_Execute, ETriggerEvent::Started, this, &AKDPlayerController::Handle_Execute);
	}
	if (IA_Aim)
	{
		EIC->BindAction(IA_Aim, ETriggerEvent::Started, this, &AKDPlayerController::Handle_AimStart);
		EIC->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AKDPlayerController::Handle_AimStop);
	}
}

void AKDPlayerController::Handle_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Movement Cancel — 공격 후반(ANS_MovementCancel 활성 중)에 이동 입력 들어오면 어빌리티 캔슬
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn))
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_MovementCanCancel))
		{
			FGameplayTagContainer CancelTags;
			CancelTags.AddTag(GameplayTags::Ability_Player_Light);
			CancelTags.AddTag(GameplayTags::Ability_Player_Heavy);
			CancelTags.AddTag(GameplayTags::Ability_Player_Dodge);
			CancelTags.AddTag(GameplayTags::Ability_Player_SprintAttack);
			ASC->CancelAbilities(&CancelTags);
		}
	}

	const FRotator YawRotation(0.0, GetControlRotation().Yaw, 0.0);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardDir, Axis.Y);
	ControlledPawn->AddMovementInput(RightDir, Axis.X);
}

void AKDPlayerController::Handle_Look(const FInputActionValue& Value)
{
	// 기능 : 마우스 입력에 감도를 곱해 시점 회전
	const FVector2D Axis = Value.Get<FVector2D>();
	
	// 조준 중엔 배율을 낮춰 정밀 조준
	float Scale = 1.f;
	float PitchSign = 1.f;   // 상하 반전 여부
	if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming))
		{
			Scale = AimSensitivityScale;
			if (bInvertAimPitch) // 조준 상하반전 활성화 시
			{
				PitchSign = -1.f;
			}
		}
	}
	AddYawInput(Axis.X * LookSensitivityYaw * Scale);
	AddPitchInput(Axis.Y * LookSensitivityPitch * Scale * PitchSign);
}

void AKDPlayerController::Handle_Jump()
{
	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (Char) Char->Jump();
}

void AKDPlayerController::Handle_StopJump()
{
	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (Char) Char->StopJumping();
}

void AKDPlayerController::Handle_SprintStart()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->StartSprint();
	
}

void AKDPlayerController::Handle_SprintStop()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->StopSprint();
	
}

void AKDPlayerController::Handle_WalkToggle()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->ToggleWalk();
}

void AKDPlayerController::Handle_LightAttack()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryLightAttack();
}

void AKDPlayerController::Handle_HeavyAttack()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryHeavyAttack();
}

// IA_Dodge (Tap) — Shift 짧게 누르면 발동. Perfect 여부는 UGA_Dodge가 자체 판단
void AKDPlayerController::Handle_Dodge()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC)PC->TryDodge();
}


void AKDPlayerController::Handle_Parry()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryParry();
}

void AKDPlayerController::Handle_ParryStop()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryParryStop();
}

void AKDPlayerController::Handle_LockOnToggle()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->ToggleLockOn();
}

void AKDPlayerController::Handle_Execute()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryExecute();
}

void AKDPlayerController::Handle_AimStart()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryAimStart();
}

void AKDPlayerController::Handle_AimStop()
{
	AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(GetPawn());
	if (PC) PC->TryAimStop();
}
