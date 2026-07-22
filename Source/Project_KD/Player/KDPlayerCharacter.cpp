#include "Player/KDPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/KDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "KDGameplayTags.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/LockOnComponent.h"
#include "Combat/WeaponComponent.h"
#include "Input/InputBufferComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/SprintComponent.h"

AKDPlayerCharacter::AKDPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Third-person back view: boom orbits behind the capsule with control rotation.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 70.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InputBuffer = CreateDefaultSubobject<UInputBufferComponent>(TEXT("InputBuffer"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	JumpMaxCount = 2;

	SprintComp = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	GunWeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("GunWeaponComponent"));
	ComboComp = CreateDefaultSubobject<UComboComponent>(TEXT("ComboComponent"));
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
	CombatStateComp = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatStateComp"));
	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

void AKDPlayerCharacter::StartSprint()
{
	if (SprintComp) SprintComp->StartSprint();
}

void AKDPlayerCharacter::StopSprint()
{
	if (SprintComp) SprintComp->StopSprint();
}

void AKDPlayerCharacter::ToggleWalk()
{
	if (SprintComp) SprintComp->ToggleWalk();
}

void AKDPlayerCharacter::ToggleLockOn()
{
	if (!LockOnComponent) return;
	LockOnComponent->ToggleLockOn();

    // 락온 상태에 맞춰 캐릭터 회전 모드 토글
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = LockOnComponent->IsLockedOn();
		MoveComp->bOrientRotationToMovement = !LockOnComponent->IsLockedOn();
	}
}

void AKDPlayerCharacter::TryConsumeAndActivate(UAbilitySystemComponent* ASC, bool bCanCancel,
                                             const FGameplayTag& InputTag, const FGameplayTag& AbilityTag)
{
	if (!InputBuffer || !ASC) return;
	if (!InputBuffer->TryConsume(InputTag)) return;

	// CancelWindow 시점에 진행 중 모든 공격/회피 GA 강제 종료 -> 즉시 새 GA 활성화.
	if (bCanCancel)
	{
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(GameplayTags::Ability_Mugong_Light);
		CancelTags.AddTag(GameplayTags::Ability_Mugong_Heavy);
		CancelTags.AddTag(GameplayTags::Ability_Mugong_Dodge);
		CancelTags.AddTag(GameplayTags::Ability_Mugong_AirCombo);
		ASC->CancelAbilities(&CancelTags);
	}

	FGameplayTagContainer ActivateTags;
	ActivateTags.AddTag(AbilityTag);
	ASC->TryActivateAbilitiesByTag(ActivateTags);
}

void AKDPlayerCharacter::ApplyMaxWalkSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = NewSpeed;
	}
}

void AKDPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AKDPlayerState* PS = GetPlayerState<AKDPlayerState>();
	if (!IsValid(PS)) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	check(ASC);

	// Owner = PlayerState (GAS authority), Avatar = Pawn (physical representation)
	ASC->InitAbilityActorInfo(PS, this);

	// Cache on base so GetAbilitySystemComponent() works from this Pawn
	AbilitySystemComponent = ASC;

	PS->GrantStartupAbilities();

	if (SprintComp)
	{
		SprintComp->OnMaxWalkSpeedChanged.AddDynamic(this, &AKDPlayerCharacter::ApplyMaxWalkSpeed);
	}
}

void AKDPlayerCharacter::TryLightAttack() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 퍼펙트 닷지 직후 -> 찌르기 
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady))
	{
		FGameplayTagContainer CounterTags;
		CounterTags.AddTag(GameplayTags::Ability_Mugong_CounterThrust);
		if (ASC->TryActivateAbilitiesByTag(CounterTags)) return;
	}

	// 공중 공격
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		FGameplayTagContainer AirTags;
		AirTags.AddTag(GameplayTags::Ability_Mugong_AirCombo);
		if (!ASC->TryActivateAbilitiesByTag(AirTags) && InputBuffer)
		{
			InputBuffer->Push(GameplayTags::Input_Action_Light); // 캔슬윈도우 밖이면 버퍼링
		}
		return; // 공중에선 스프린트/지상 Light 공격 진행 X
	}
	
	// 달리기 공격
	if (IsSprinting())
	{
		FGameplayTagContainer SprintTags;
		SprintTags.AddTag(GameplayTags::Ability_Mugong_SprintAttack);
		if (ASC->TryActivateAbilitiesByTag(SprintTags)) return;
	}

	// D8 P2 — symmetric. If mid-dodge but in cancel window, manually cancel Dodge first.
	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);
	if (bDodging && bCanCancel)
	{
		FGameplayTagContainer DodgeTags;
		DodgeTags.AddTag(GameplayTags::Ability_Mugong_Dodge);
		ASC->CancelAbilities(&DodgeTags);
	}

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(GameplayTags::Ability_Mugong_Light);

	// Buffer only if activation was blocked (mid-combo or mid-dodge without cancel window).
	const bool bActivated = ASC->TryActivateAbilitiesByTag(ActivationTags);
	
	if (!bActivated && InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Light);
	}
}

void AKDPlayerCharacter::TryHeavyAttack() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);
	if (bDodging && bCanCancel)
	{
		FGameplayTagContainer DodgeTags;
		DodgeTags.AddTag(GameplayTags::Ability_Mugong_Dodge);
		ASC->CancelAbilities(&DodgeTags);
	}

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(GameplayTags::Ability_Mugong_Heavy);

	if (!ASC->TryActivateAbilitiesByTag(ActivationTags) && InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Heavy);
	}
}

void AKDPlayerCharacter::TryParry() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(GameplayTags::Ability_Mugong_Parry);
	ASC->TryActivateAbilitiesByTag(ActivationTags);
}

void AKDPlayerCharacter::TryParryStop() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(GameplayTags::Ability_Mugong_Parry);
	ASC->CancelAbilities(&CancelTags);
}

void AKDPlayerCharacter::TryExecute() const
{
	AActor* Target = nullptr;
	if (LockOnComponent)
	{
		Target = LockOnComponent->IsLockedOn()
			? LockOnComponent->GetLockedTarget()
			: LockOnComponent->FindBestTarget();
	}
	if (!Target) { return; }

	// 거리 밖에 있으면 처형 발동 불가
	if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > ExecutionRange)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC
		|| !TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Staggered))
	{
		return;
	}

	FGameplayEventData Data;
	Data.Instigator = this;
	Data.InstigatorTags.AddTag(GameplayTags::Ability_Mugong_Execution);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Target, GameplayTags::Event_Combat_Hit, Data);
}

void AKDPlayerCharacter::TryDodge() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// D8 P2 — if mid-attack but in cancel window, manually cancel the current attack first.
	// Dodge GA has ActivationBlockedTags(Attacking), so TryActivate would otherwise fail.
	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);

	if (bDodging && !bCanCancel)
	{
		return;
	}
	
	if (bAttacking && bCanCancel)
	{
		FGameplayTagContainer LightTags;
		LightTags.AddTag(GameplayTags::Ability_Mugong_Light);
		ASC->CancelAbilities(&LightTags);
	}

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(GameplayTags::Ability_Mugong_Dodge);

	// D8 P4 — buffer when blocked so Tick re-dispatches on Attacking-tag clear or CanCancel arrival.
	if (!ASC->TryActivateAbilitiesByTag(ActivationTags) && InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Dodge);
	}
}

void AKDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TurnSpeedCurve)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			const float Speed = GetVelocity().Size2D();
			const float TurnRate = TurnSpeedCurve->GetFloatValue(Speed);
			Move->RotationRate = FRotator(0.f, TurnRate, 0.f);
		}
	}
	
	if (!InputBuffer) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const FVector Accel = Move->GetCurrentAcceleration(); // 지금 누르고 있는 입력 방향
		const bool bGrounded = !Move->IsFalling(); // 땅 유무
		const bool bFree = !ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn); // 락온 X
		const bool bTurning = ASC->HasMatchingGameplayTag(GameplayTags::State_Movement_Turning); // 이미 턴 X
		const bool bInCombat = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InCombat); // 전투모드 X

		// b변수 4개 + 입력 있는지, 현재 움직이는 지
		if (bGrounded && bFree && !bTurning && !bInCombat && !Accel.IsNearlyZero() && GetVelocity().Size2D() > 50.f)
		{
			// 지금 메시가 보는 방향(ActorYaw)과 가고 싶은 방향(입력Yaw)의 부호있는 각도 차.
			// 0=정면, +-180=정반대. FindDeltaAngleDegrees가 -180~180으로 정규화해줌.
			const float Angle = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, Accel.Rotation().Yaw);
			if (FMath::Abs(Angle) >= 135.f)
			{
				FGameplayTagContainer TurnTags;
				TurnTags.AddTag(GameplayTags::Ability_Movement_Turn);
				ASC->TryActivateAbilitiesByTag(TurnTags);
			}
		}
	}

	// 락온 회전 모드 체크
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const bool bLocked = LockOnComponent && LockOnComponent->IsLockedOn();
		Move->bUseControllerDesiredRotation = bLocked;   
		Move->bOrientRotationToMovement = !bLocked;      
	}

	const bool bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);

	
	// D8 P4 — dispatch buffered Dodge when player exits Attacking, OR when cancel window opens.
	if (!bAttacking || bCanCancel)
	{
		if (InputBuffer->TryConsume(GameplayTags::Input_Action_Dodge))
		{
			if (bAttacking && bCanCancel)
			{
				FGameplayTagContainer LightTags;
				LightTags.AddTag(GameplayTags::Ability_Mugong_Light);
				ASC->CancelAbilities(&LightTags);
			}
			FGameplayTagContainer DodgeTags;
			DodgeTags.AddTag(GameplayTags::Ability_Mugong_Dodge);
			ASC->TryActivateAbilitiesByTag(DodgeTags);
		}
	}

	// Symmetric — dispatch buffered LightAttack when player exits Dodging, OR when cancel window opens.
	
	// 콤보 — 공격/회피 둘 다 아닐 때 OR CancelWindow 열렸을 때만 Consume.
	if ((!bAttacking && !bDodging) || bCanCancel)
	{
		const bool bAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
		if (bAir)
		{
			// 공중: 버퍼된 Light를 AirCombo로 변경
			TryConsumeAndActivate(ASC, bCanCancel,
				GameplayTags::Input_Action_Light, GameplayTags::Ability_Mugong_AirCombo);
		}
		else
		{
			TryConsumeAndActivate(ASC, bCanCancel, GameplayTags::Input_Action_Light, GameplayTags::Ability_Mugong_Light);
			TryConsumeAndActivate(ASC, bCanCancel, GameplayTags::Input_Action_Heavy, GameplayTags::Ability_Mugong_Heavy);
		}
	}
}

void AKDPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// 공중(falling) 진입/착지 시 InAir 태그 토글 
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
		ASC->AddLooseGameplayTag(GameplayTags::State_Movement_InAir);
	else
		ASC->RemoveLooseGameplayTag(GameplayTags::State_Movement_InAir);
}

bool AKDPlayerCharacter::IsSprinting() const
{
	return SprintComp ? SprintComp->IsSprinting() : false;
}

bool AKDPlayerCharacter::IsFullSprinting() const
{
	return SprintComp ? SprintComp->IsFullSprinting() : false;
}

bool AKDPlayerCharacter::IsWalking() const
{
	return SprintComp ? SprintComp->IsWalking() : false;
}

bool AKDPlayerCharacter::ActivateByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const
{
	FGameplayTagContainer C;
	C.AddTag(Tag); return ASC->TryActivateAbilitiesByTag(C);
}

void AKDPlayerCharacter::CancelByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const
{
	FGameplayTagContainer C;
	C.AddTag(Tag); return ASC->CancelAbilities(&C);
}