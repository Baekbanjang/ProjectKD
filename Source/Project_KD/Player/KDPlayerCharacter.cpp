#include "Player/KDPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/KDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "KDSpringArmComponent.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HitStopComponent.h"
#include "Combat/LockOnComponent.h"
#include "Combat/WeaponComponent.h"
#include "Components/SplineComponent.h"
#include "Input/InputBufferComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/SprintComponent.h"
#include "Player/KDPlayerAbilityInputComponent.h"


AKDPlayerCharacter::AKDPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 요 = 컨트롤러 / 상하 = 스플라인
	CameraBoom = CreateDefaultSubobject<UKDSpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bInheritPitch = false; // 스프링암 길이 = 수평 거리

	CameraDollySpline = CreateDefaultSubobject<USplineComponent>(TEXT("CameraDollySpline"));
	CameraDollySpline->SetupAttachment(RootComponent);
	CameraDollySpline->ClearSplinePoints(false); // 포인트 초기화
	
	// SB 원본 스플라인 3점 - 위치 | 도착 탄젠트 | 출발 탄젠트 (탄젠트 수동 지정)
	CameraDollySpline->AddPoints({
		FSplinePoint(0.f, FVector(  -1.21f,  0.f, 514.18f), FVector(-594.58f, 0.f, -110.55f), FVector(-594.58f, 0.f, -110.55f)),   // 마우스 -89.0도 · 거리 514
		FSplinePoint(1.f, FVector(-382.54f, 40.f, 117.58f), FVector(   1.27f, 0.f, -607.56f), FVector(   1.27f, 0.f, -607.56f)),   // 마우스 -24.8도 · 거리 400
		FSplinePoint(2.f, FVector( -52.91f,  0.f, -82.98f), FVector(  57.84f, 0.f,   -0.34f), FVector(  57.84f, 0.f,   -0.34f)),   // 마우스 +45.0도 · 거리  98
	}, false);
	CameraDollySpline->UpdateSpline();
	CameraBoom->DollySpline = CameraDollySpline;

	
	AimDollySpline = CreateDefaultSubobject<USplineComponent>(TEXT("AimDollySpline"));
	AimDollySpline->SetupAttachment(RootComponent);
	AimDollySpline->ClearSplinePoints(false);

	// 조준 스플라인 3점
	AimDollySpline->AddPoints({
		FSplinePoint(0.f, FVector(-180.f, 65.f,  90.f)),   // 마우스 위
		FSplinePoint(1.f, FVector(-190.f, 65.f,  20.f)),   // 정면
		FSplinePoint(2.f, FVector(-180.f, 65.f, -50.f)),   // 마우스 아래
	}, false);
	AimDollySpline->UpdateSpline();
	CameraBoom->AimDollySpline = AimDollySpline;
	
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
	AbilityInputComp = CreateDefaultSubobject<UKDPlayerAbilityInputComponent>(TEXT("AbilityInputComponent"));
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
	CombatStateComp = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatStateComp"));
	HitStopComp = CreateDefaultSubobject<UHitStopComponent>(TEXT("HitStopComp"));
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

void AKDPlayerCharacter::ApplyMaxWalkSpeed(float NewSpeed)
{
	BaseWalkSpeed = NewSpeed;
	RefreshMaxWalkSpeed();
}

void AKDPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AKDPlayerState* PS = GetPlayerState<AKDPlayerState>();
	if (!IsValid(PS)) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	check(ASC);

	// Owner = PlayerState(GAS 권한), Avatar = Pawn(실제 피지컬)
	ASC->InitAbilityActorInfo(PS, this);

	// 베이스에 캐시 — 해당 Pawn에서 GetAbilitySystemComponent() 가능
	AbilitySystemComponent = ASC;

	PS->GrantStartupAbilities();

	if (SprintComp)
	{
		SprintComp->OnMaxWalkSpeedChanged.AddDynamic(this, &AKDPlayerCharacter::ApplyMaxWalkSpeed);
	}
}

// 아래 Try* 8개 = BP 호환용 위임. 실제 판단은 KDPlayerAbilityInputComponent
void AKDPlayerCharacter::TryLightAttack() const
{
	if (AbilityInputComp) AbilityInputComp->TryLightAttack();
}

void AKDPlayerCharacter::TryHeavyAttack() const
{
	if (AbilityInputComp) AbilityInputComp->TryHeavyAttack();
}

void AKDPlayerCharacter::TryParry() const
{
	if (AbilityInputComp) AbilityInputComp->TryParry();
}

void AKDPlayerCharacter::TryParryStop() const
{
	if (AbilityInputComp) AbilityInputComp->TryParryStop();
}

void AKDPlayerCharacter::TryExecute() const
{
	if (AbilityInputComp) AbilityInputComp->TryExecute();
}

void AKDPlayerCharacter::TryAimStart() const
{
	if (AbilityInputComp) AbilityInputComp->TryAimStart();
}

void AKDPlayerCharacter::TryAimStop() const
{
	if (AbilityInputComp) AbilityInputComp->TryAimStop();
}

void AKDPlayerCharacter::TryDodge() const
{
	if (AbilityInputComp) AbilityInputComp->TryDodge();
}

void AKDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 회전 속도 조절 — 빠르게 달릴수록 천천히 돌게
	if (TurnSpeedCurve)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			const float Speed = GetVelocity().Size2D();
			const float TurnRate = TurnSpeedCurve->GetFloatValue(Speed);
			Move->RotationRate = FRotator(0.f, TurnRate, 0.f);
		}
	}

	// 가드 — 아래는 ASC가 있어야 도는 로직
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 락온 | 조준 회전 모드 — 락온이면 타겟 방향, 아니면 카메라 방향
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const bool bLocked = LockOnComponent && LockOnComponent->IsLockedOn();
		const bool bAiming = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming);

		// 둘중 하나면 카메라 방향
		const bool bFaceCamera = bLocked || bAiming;
		Move->bUseControllerDesiredRotation = bFaceCamera;   
		Move->bOrientRotationToMovement = !bFaceCamera;
		RefreshMaxWalkSpeed(); 
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

float AKDPlayerCharacter::GetCameraRailAlpha() const
{
	return CameraBoom ? CameraBoom->GetRailAlpha() : 0.f;
}

bool AKDPlayerCharacter::IsSprinting() const
{
	return SprintComp ? SprintComp->IsSprinting() : false;
}

bool AKDPlayerCharacter::IsFullSprinting() const
{
	return SprintComp ? SprintComp->IsFullSprinting() : false;
}

void AKDPlayerCharacter::RefreshMaxWalkSpeed()
{
	// 기능 : 락온 | 조준 감속을 반영해서 MaxWalkSpeed 갱신

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	// 감속 조건 겹치면 더 느린 쪽
	float Speed = BaseWalkSpeed;
	
	if (LockOnComponent && LockOnComponent->IsLockedOn())
	{
		Speed = FMath::Min(Speed, LockOnMoveSpeed);
	}

	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming))
		{
			Speed = FMath::Min(Speed, AimMoveSpeed);
		}
	}

	Move->MaxWalkSpeed = Speed;
}
