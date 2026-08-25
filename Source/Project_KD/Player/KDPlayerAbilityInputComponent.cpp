// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/KDPlayerAbilityInputComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "Combat/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/InputBufferComponent.h"
#include "Movement/SprintComponent.h"


// 헬퍼
namespace
{
	// 콤보 게이트 판단에 반복되는 태그 3종 — 한 곳에서 조회
	struct FComboGateState
	{
		bool bAttacking = false;
		bool bDodging = false;
		bool bCanCancel = false;
	};

	FComboGateState QueryComboGateState(const UAbilitySystemComponent* ASC)
	{
		FComboGateState State;
		State.bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
		State.bDodging   = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Dodging);
		State.bCanCancel = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CanCancel);
		return State;
	}
}

void UKDPlayerAbilityInputComponent::BeginPlay()
{
	// 기능 : 형제 컴포넌트 1회 조회 — 전부 소유 액터 생성자 소속
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		InputBuffer = Owner->FindComponentByClass<UInputBufferComponent>();
		ComboComp   = Owner->FindComponentByClass<UComboComponent>();
		LockOnComp  = Owner->FindComponentByClass<ULockOnComponent>();
		SprintComp  = Owner->FindComponentByClass<USprintComponent>();
	}
}

UAbilitySystemComponent* UKDPlayerAbilityInputComponent::GetASC() const
{
	// 기능 : ASC 조회 — PlayerState 소속이라 BeginPlay 시점엔 없을 수 있어 매번 조회
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

void UKDPlayerAbilityInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// 기능 : 제자리 턴 판단 + 버퍼 사용
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!InputBuffer) return;

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	UpdateTurnInPlace(ASC);
	ConsumeBufferedInput(ASC);
}

UKDPlayerAbilityInputComponent::UKDPlayerAbilityInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MovementCancelableTags.AddTag(GameplayTags::Ability_Player_Light);
	MovementCancelableTags.AddTag(GameplayTags::Ability_Player_Heavy);
	MovementCancelableTags.AddTag(GameplayTags::Ability_Player_Dodge);
	MovementCancelableTags.AddTag(GameplayTags::Ability_Player_SprintAttack);

	AttackCancelableTags.AddTag(GameplayTags::Ability_Player_Light);
	AttackCancelableTags.AddTag(GameplayTags::Ability_Player_Heavy);
	AttackCancelableTags.AddTag(GameplayTags::Ability_Player_SprintAttack);
	AttackCancelableTags.AddTag(GameplayTags::Ability_Player_CounterThrust);
}

void UKDPlayerAbilityInputComponent::UpdateTurnInPlace(UAbilitySystemComponent* ASC) const
{
	// 기능 : 가던 방향 반대 입력이면 턴 어빌리티 — 상태 조건은 GA_Turn 의 ActivationBlockedTags 담당
	const ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	const UCharacterMovementComponent* Move = Owner->GetCharacterMovement();
	if (!Move) return;

	// 지금 누르고 있는 입력 방향
	const FVector Accel = Move->GetCurrentAcceleration();
	if (Accel.IsNearlyZero() || Owner->GetVelocity().Size2D() <= 50.f) return;

	// 메시가 보는 방향과 가려는 방향의 부호있는 각도 차 — 0 = 정면 | 180 = 정반대
	const float Angle = FMath::FindDeltaAngleDegrees(Owner->GetActorRotation().Yaw, Accel.Rotation().Yaw);
	if (FMath::Abs(Angle) < 135.f) return;

	FGameplayTagContainer TurnTags;
	TurnTags.AddTag(GameplayTags::Ability_Movement_Turn);
	ASC->TryActivateAbilitiesByTag(TurnTags);
}

void UKDPlayerAbilityInputComponent::ConsumeBufferedInput(UAbilitySystemComponent* ASC) const
{
	// 기능 : 버퍼된 회피 | 공격 꺼내기
	const FComboGateState Gate = QueryComboGateState(ASC);

	// 회피 — 공격이 끝났거나 캔슬 윈도우가 열렸을 때만
	if (!Gate.bAttacking || Gate.bCanCancel)
	{
		// TryConsume — 버퍼에 있으면 꺼내면서 지움
		if (InputBuffer->TryConsume(GameplayTags::Input_Action_Dodge))
		{
			// GA_Dodge 는 Attacking 태그가 있으면 활성화 X — 공격부터 종료시켜 태그 제거
			if (Gate.bAttacking && Gate.bCanCancel)
			{
				ASC->CancelAbilities(&AttackCancelableTags);
			}

			FGameplayTagContainer DodgeTags;
			DodgeTags.AddTag(GameplayTags::Ability_Player_Dodge);

			// 버퍼 경유로 켜져도 콤보 위치는 동일하게 이동
			if (ASC->TryActivateAbilitiesByTag(DodgeTags) && ComboComp)
			{
				const bool bPerfect = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady);
				ComboComp->EnterEvadeNode(bPerfect);
			}
		}
	}

	// 공격 — 회피 쪽과 대칭
	if ((!Gate.bAttacking && !Gate.bDodging) || Gate.bCanCancel)
	{
		const ACharacter* Owner = Cast<ACharacter>(GetOwner());
		const UCharacterMovementComponent* Move = Owner ? Owner->GetCharacterMovement() : nullptr;
		const bool bAir = Move && Move->IsFalling();

		if (bAir)
		{
			// 공중 — 버퍼된 Light 를 AirCombo 로 교체해 발동
			TryConsumeAndActivate(ASC, Gate.bCanCancel,
				GameplayTags::Input_Action_Light, GameplayTags::Ability_Player_AirCombo);
		}
		else
		{
			// 지상 — 콤보 노드 이동은 GA 담당
			TryConsumeAndActivate(ASC, Gate.bCanCancel, GameplayTags::Input_Action_Light, GameplayTags::Ability_Player_Light);
			TryConsumeAndActivate(ASC, Gate.bCanCancel, GameplayTags::Input_Action_Heavy, GameplayTags::Ability_Player_Heavy);
		}
	}
}

void UKDPlayerAbilityInputComponent::TryLightAttack() const
{
	// 기능 : 상황별 약공 분기 — 조준 | 카운터 | 공중 | 스프린트 | 지상
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	// 조준 중이면 근접 대신 사격
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming))
	{
		ActivateByTag(ASC, GameplayTags::Ability_Player_Shoot);
		return;   // 발사 실패해도 근접 공격 X
	}

	// 퍼펙트 닷지 직후 = 찌르기
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady))
	{
		if (ActivateByTag(ASC, GameplayTags::Ability_Player_CounterThrust)) return;
	}

	const ACharacter* Owner = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Move = Owner ? Owner->GetCharacterMovement() : nullptr;

	// 공중 공격
	if (Move && Move->IsFalling())
	{
		// 캔슬 윈도우 밖이면 버퍼링
		if (!ActivateByTag(ASC, GameplayTags::Ability_Player_AirCombo) && InputBuffer)
		{
			InputBuffer->Push(GameplayTags::Input_Action_Light);
		}
		return;   // 공중에선 스프린트 | 지상 Light 진행 X
	}

	// 달리기 공격
	if (SprintComp && SprintComp->IsSprinting())
	{
		if (ActivateByTag(ASC, GameplayTags::Ability_Player_SprintAttack)) return;
	}

	// 회피 중이어도 캔슬 윈도우면 회피부터 끊고 공격 — 회피 쪽 처리와 대칭
	const FComboGateState Gate = QueryComboGateState(ASC);
	if (Gate.bDodging && Gate.bCanCancel)
	{
		CancelByTag(ASC, GameplayTags::Ability_Player_Dodge);
	}

	// 활성화가 막혔을 때만 버퍼에 저장 — 콤보 진행 중이거나 캔슬 윈도우 밖 회피 중
	if (!ActivateByTag(ASC, GameplayTags::Ability_Player_Light) && InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Light);
	}
}

void UKDPlayerAbilityInputComponent::TryHeavyAttack() const
{
	// 기능 : 강공 — 캔슬 윈도우면 회피를 끊고 발동
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	const FComboGateState Gate = QueryComboGateState(ASC);
	if (Gate.bDodging && Gate.bCanCancel)
	{
		CancelByTag(ASC, GameplayTags::Ability_Player_Dodge);
	}

	if (!ActivateByTag(ASC, GameplayTags::Ability_Player_Heavy) && InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Heavy);
	}
}

void UKDPlayerAbilityInputComponent::TryParry() const
{
	// 기능 : 가드 시작
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ActivateByTag(ASC, GameplayTags::Ability_Player_Parry);
	}
}

void UKDPlayerAbilityInputComponent::TryParryStop() const
{
	// 기능 : 가드 해제
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		CancelByTag(ASC, GameplayTags::Ability_Player_Parry);
	}
}

void UKDPlayerAbilityInputComponent::TryDodge() const
{
	// 기능 : 회피 — 캔슬 윈도우면 공격을 끊고 발동, 실패 시 버퍼링
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	// GA_Dodge 는 ActivationBlockedTags(Attacking) 때문에 공격 중 활성화 X
	// 캔슬 윈도우면 공격을 먼저 끊어 태그를 없앤 뒤 활성화
	const FComboGateState Gate = QueryComboGateState(ASC);

	if (Gate.bDodging && !Gate.bCanCancel) return;

	if (Gate.bAttacking && Gate.bCanCancel)
	{
		ASC->CancelAbilities(&AttackCancelableTags);
	}

	// 활성화 실패 시 버퍼로 — Attacking 태그 해제 또는 캔슬 윈도우 개방 시 Tick 이 재시도
	if (ActivateByTag(ASC, GameplayTags::Ability_Player_Dodge))
	{
		// 회피도 콤보 노드 — 퍼펙트는 별도 노드
		if (ComboComp)
		{
			const bool bPerfect = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady);
			ComboComp->EnterEvadeNode(bPerfect);
		}
	}
	else if (InputBuffer)
	{
		InputBuffer->Push(GameplayTags::Input_Action_Dodge);
	}
}

void UKDPlayerAbilityInputComponent::TryExecute() const
{
	// 기능 : 사거리 안 경직 대상에 처형 이벤트 전송
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !LockOnComp) return;

	AActor* Target = LockOnComp->IsLockedOn()
		? LockOnComp->GetLockedTarget()
		: LockOnComp->FindBestTarget();
	if (!Target) return;

	// 사거리 밖이면 처형 발동 X
	if (FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation()) > ExecutionRange) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Staggered)) return;

	// 처형 판정은 대상의 ExecutionComponent 담당
	FGameplayEventData Data;
	Data.Instigator = Owner;
	Data.InstigatorTags.AddTag(GameplayTags::Ability_Player_Execution);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, GameplayTags::Event_Combat_Hit, Data);
}

void UKDPlayerAbilityInputComponent::TryAimStart() const
{
	// 기능 : 조준 시작
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ActivateByTag(ASC, GameplayTags::Ability_Player_Aim);
	}
}

void UKDPlayerAbilityInputComponent::TryAimStop() const
{
	// 기능 : 조준 해제
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		CancelByTag(ASC, GameplayTags::Ability_Player_Aim);
	}
}

void UKDPlayerAbilityInputComponent::TryMovementCancel() const
{
	// 기능 : 이동 입력이 공격 후반(MovementCanCancel 창) 어빌리티를 끊음
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;

	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_MovementCanCancel))
	{
		ASC->CancelAbilities(&MovementCancelableTags);
	}
}

void UKDPlayerAbilityInputComponent::TryConsumeAndActivate(UAbilitySystemComponent* ASC, bool bCanCancel,
	const FGameplayTag& InputTag, const FGameplayTag& AbilityTag) const
{
	// 기능 : 버퍼에 있으면 꺼내 어빌리티 발동
	if (!InputBuffer || !ASC) return;
	if (!InputBuffer->TryConsume(InputTag)) return;

	// 캔슬 윈도우면 진행 중 공격 | 회피 GA 강제 종료 -> 즉시 새 GA 활성화
	if (bCanCancel)
	{
		FGameplayTagContainer CancelTags = AttackCancelableTags;
		CancelTags.AddTag(GameplayTags::Ability_Player_Dodge);     // 회피 후딜에서 공격으로 연결
		CancelTags.AddTag(GameplayTags::Ability_Player_AirCombo);  // 공중 콤보 사이 교체
		ASC->CancelAbilities(&CancelTags);
	}

	FGameplayTagContainer ActivateTags;
	ActivateTags.AddTag(AbilityTag);
	ASC->TryActivateAbilitiesByTag(ActivateTags);
}

bool UKDPlayerAbilityInputComponent::ActivateByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const
{
	// 기능 : 태그 1개로 어빌리티 활성화
	FGameplayTagContainer C;
	C.AddTag(Tag);
	return ASC->TryActivateAbilitiesByTag(C);
}

void UKDPlayerAbilityInputComponent::CancelByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const
{
	// 기능 : 태그 1개로 어빌리티 취소
	FGameplayTagContainer C;
	C.AddTag(Tag);
	ASC->CancelAbilities(&C);
}
