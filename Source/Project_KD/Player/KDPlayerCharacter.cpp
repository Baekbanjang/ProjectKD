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

// 헬퍼
namespace
{
	// 캔슬 윈도우에서 끊을 수 있는 공격 GA 목록
	const FGameplayTagContainer& GetCancelableAttackTags()
	{
		static const FGameplayTagContainer Tags = []
		{
			FGameplayTagContainer C;
			C.AddTag(GameplayTags::Ability_Mugong_Light);
			C.AddTag(GameplayTags::Ability_Mugong_Heavy);
			C.AddTag(GameplayTags::Ability_Mugong_SprintAttack);
			C.AddTag(GameplayTags::Ability_Mugong_CounterThrust);
			return C;
		}();
		return Tags;
	}
}


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

void AKDPlayerCharacter::TryConsumeAndActivate(UAbilitySystemComponent* ASC, bool bCanCancel,
                                             const FGameplayTag& InputTag, const FGameplayTag& AbilityTag)
{
	if (!InputBuffer || !ASC) return;
	if (!InputBuffer->TryConsume(InputTag)) return;

	// CancelWindow 시점에 진행 중 모든 공격/회피 GA 강제 종료 -> 즉시 새 GA 활성화.
	if (bCanCancel)
	{
		FGameplayTagContainer CancelTags = GetCancelableAttackTags();
		CancelTags.AddTag(GameplayTags::Ability_Mugong_Dodge);     // 회피 후딜에서 공격으로 잇기
		CancelTags.AddTag(GameplayTags::Ability_Mugong_AirCombo);  // 공중 콤보 사이 갈아타기
		ASC->CancelAbilities(&CancelTags);
	}

	FGameplayTagContainer ActivateTags;
	ActivateTags.AddTag(AbilityTag);
	ASC->TryActivateAbilitiesByTag(ActivateTags);
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

void AKDPlayerCharacter::TryLightAttack() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 조준 중이면 근접 대신 사격
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming))
	{
		FGameplayTagContainer ShootTags;
		ShootTags.AddTag(GameplayTags::Ability_Mugong_Shoot);
		ASC->TryActivateAbilitiesByTag(ShootTags);
		return; // 발사 실패해도 근접 공격 X
	}

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

	// 회피 중이어도 캔슬 윈도우면 회피부터 끊고 공격 — 회피 쪽 처리와 대칭
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

	// 켜기 막혔을 때만 버퍼에 저장 — 콤보 진행 중이거나 캔슬 윈도우 밖 회피 중
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

void AKDPlayerCharacter::TryAimStart() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ActivateByTag(ASC, GameplayTags::Ability_Mugong_Aim);
	}
}

void AKDPlayerCharacter::TryAimStop() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		CancelByTag(ASC, GameplayTags::Ability_Mugong_Aim);
	}
}

void AKDPlayerCharacter::TryDodge() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// GA_Dodge는 ActivationBlockedTags(Attacking) 때문에 공격 중엔 못 켜짐
	// 캔슬 윈도우면 공격을 먼저 끊어서 태그를 없앤 뒤 켠다
	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);

	if (bDodging && !bCanCancel)
	{
		return;
	}
	
	if (bAttacking && bCanCancel)
	{
		ASC->CancelAbilities(&GetCancelableAttackTags());
	}

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(GameplayTags::Ability_Mugong_Dodge);

	// 켜기 실패하면 버퍼로 — Attacking 태그가 풀리거나 캔슬 윈도우 열릴 때 Tick이 다시 시도
	if (ASC->TryActivateAbilitiesByTag(ActivationTags))
	{
		// 회피도 콤보 노드 — 퍼펙트면 별도 노드
		if (ComboComp)
		{
			const bool bPerfect = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady);
			ComboComp->EnterNode(bPerfect ? TEXT("JustEvade") : TEXT("Evade"), 0.8f);
		}
	}
	// 회피가 안켜졌으면
	else if (InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Dodge);
	}
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

	// 가드 — 아래는 전부 버퍼/ASC가 있어야 도는 로직
	if (!InputBuffer) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 제자리 턴 — 가던 방향 반대로 입력하면 Turn 어빌리티. 상태 조건은 GA_Turn의 ActivationBlockedTags 담당
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const FVector Accel = Move->GetCurrentAcceleration(); // 지금 누르고 있는 입력 방향
		if (!Accel.IsNearlyZero() && GetVelocity().Size2D() > 50.f)
		{
			// 지금 메시가 보는 방향(ActorYaw)과 가고 싶은 방향(입력Yaw)의 부호있는 각도 차
			// 0=정면, +-180=정반대. FindDeltaAngleDegrees가 -180~180으로 정규화
			const float Angle = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, Accel.Rotation().Yaw);
			if (FMath::Abs(Angle) >= 135.f)
			{
				FGameplayTagContainer TurnTags;
				TurnTags.AddTag(GameplayTags::Ability_Movement_Turn);
				ASC->TryActivateAbilitiesByTag(TurnTags);
			}
		}
	}

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
	
	// 버퍼를 사용하기 위한 현재 상태
	const bool bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
	const bool bDodging = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
	const bool bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);


	// 버퍼된 회피 꺼내기 — 공격이 끝났거나 캔슬 윈도우 열렸을 때만
	if (!bAttacking || bCanCancel)
	{
		// TryConsume — 버퍼에 있으면 꺼내면서 지움, 0.2초 지난 입력은 이미 만료
		if (InputBuffer->TryConsume(GameplayTags::Input_Action_Dodge))
		{
			// GA_Dodge는 Attacking 태그 있으면 못 켜짐 — 공격부터 종료시켜 태그 제거
			if (bAttacking && bCanCancel)
			{
				ASC->CancelAbilities(&GetCancelableAttackTags());
			}
			FGameplayTagContainer DodgeTags;
			DodgeTags.AddTag(GameplayTags::Ability_Mugong_Dodge);

			// 버퍼 경유로 켜져도 콤보 위치는 똑같이 옮김
			if (ASC->TryActivateAbilitiesByTag(DodgeTags) && ComboComp)
			{
				const bool bPerfect = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady);
				ComboComp->EnterNode(bPerfect ? TEXT("JustEvade") : TEXT("Evade"), 0.8f);
			}
		}
	}

	// 버퍼된 공격 꺼내기 — 회피 쪽과 대칭, 공격/회피 둘 다 아닐 때 OR 캔슬 윈도우 열렸을 때만
	if ((!bAttacking && !bDodging) || bCanCancel)
	{
		const bool bAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
		if (bAir)
		{
			// 공중 — 버퍼된 Light를 AirCombo로 바꿔서 발동
			TryConsumeAndActivate(ASC, bCanCancel,
				GameplayTags::Input_Action_Light, GameplayTags::Ability_Mugong_AirCombo);
		}
		else
		{
			// 지상 — 약공/강공 각각 버퍼 확인 후 발동, 콤보 노드 이동은 GA가 담당
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
