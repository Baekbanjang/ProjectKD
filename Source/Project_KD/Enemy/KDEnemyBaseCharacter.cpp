// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/KDEnemyBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "AbilitySystem/Attributes/AS_Combat.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/HitFeedbackComponent.h"
#include "Combat/StaggerComponent.h"
#include "Combat/ExecutionComponent.h"
#include "Combat/ExecutionProfile.h"
#include "Enemy/EnemyDefinitionDataAsset.h"
#include "Enemy/AI/EncounterSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "KDGameplayTags.h"
#include "Enemy/KDEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Perception/AISense_Damage.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "UI/KDEnemyStateBarWidget.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
// 개발용 넉백 표시 스위치 — 콘솔 KD.ShowKnock 1
static TAutoConsoleVariable<int32> CVarShowKnock(
	TEXT("KD.ShowKnock"), 0,
	TEXT("넉백 배수·속도·거리 온스크린 표시 유무"), ECVF_Cheat);
#endif

AKDEnemyBaseCharacter::AKDEnemyBaseCharacter()
{
	// ASC = Pawn 직접 소유 (플레이어는 PlayerState)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CharacterAttributes = CreateDefaultSubobject<UAS_CharacterBase>(TEXT("CharacterAttributes"));
	CombatAttributes = CreateDefaultSubobject<UAS_Combat>(TEXT("CombatAttributes"));

	// Minimal 모드 — 큐만 복제
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HitFeedback = CreateDefaultSubobject<UHitFeedbackComponent>(TEXT("HitFeedback"));

	// 경직·처형 = 전용 컴포넌트 — 각자 BeginPlay 에서 ASC 캐시 + 이벤트 구독
	StaggerComp = CreateDefaultSubobject<UStaggerComponent>(TEXT("StaggerComp"));
	ExecutionComp = CreateDefaultSubobject<UExecutionComponent>(TEXT("ExecutionComp"));

	// 상태 바 — 위젯 클래스 지정 = BP
	StateBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StateBarWidget"));
	StateBarWidget->SetupAttachment(RootComponent);
	StateBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	StateBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	StateBarWidget->SetDrawAtDesiredSize(true);
	StateBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StateBarWidget->SetVisibility(false);
	
	// 배치·스폰 적의 AIController 자동 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AKDEnemyAIController::StaticClass();

	// 전투 facing = controller desired rotation — SetFocus·ClearFocus 관리 = BTService_FindPlayer
	// BP child 에서 아래 회전 플래그 직접 수정 X
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = true;
		Move->bOrientRotationToMovement = false;
		Move->RotationRate = FRotator(0.f, 360.f, 0.f);     // 턴속도 (도/초) — PossessedBy 에서 DataAsset 값으로 덮음
	}
}

void AKDEnemyBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!IsValid(AbilitySystemComponent)) return;

	// Owner = Avatar = this Pawn (PlayerState 미사용)
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 진영 태그 — WeaponTrace GA 의 아군 사격 방지용
	AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::Team_Enemy);

	// 적별 스탯·StartupAbilities = EnemyDefinition
	if (ensureMsgf(EnemyDefinition != nullptr, TEXT("EnemyDefinition unset on %s — using AttributeSet defaults"), *GetName()))
	{
		// 델리게이트 바인딩 전에 스탯 set — 순서 고정
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetMaxHealthAttribute(), EnemyDefinition->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetHealthAttribute(),    EnemyDefinition->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetMaxPoiseAttribute(),  EnemyDefinition->MaxPoise);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetPoiseAttribute(),     EnemyDefinition->MaxPoise);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_Combat::GetAttackPowerAttribute(),      EnemyDefinition->AttackPower);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_Combat::GetDefenseAttribute(),          EnemyDefinition->Defense);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetMaxShieldAttribute(), EnemyDefinition->MaxShield);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetShieldAttribute(),    EnemyDefinition->MaxShield);


		// 싱글 전제 — 멀티 전환 시 HasAuthority 게이트 필요
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : EnemyDefinition->StartupAbilities)
		{
			if (!AbilityClass) continue;
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}

		// DataAsset 턴속도 적용 — 생성자 기본값 360 을 덮음
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->RotationRate.Yaw = GetTurnRate();
		}
	}

	// Pawn 구독 = Health -> 사망 / Poise -> StaggerComp / 처형 -> ExecutionComp
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAS_CharacterBase::GetHealthAttribute())
		.AddUObject(this, &AKDEnemyBaseCharacter::OnHealthChanged);

	// Poise 차감 + 넉백 = Pawn / 처형 트리거 = ExecutionComp 별도 구독
	AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(GameplayTags::Event_Combat_Hit)
		.AddUObject(this, &AKDEnemyBaseCharacter::OnHitReceived);

	// 컴포넌트 배선 = 델리게이트
	if (StaggerComp)
	{
		StaggerComp->OnStaggerBegin.AddDynamic(this, &AKDEnemyBaseCharacter::OnStaggerBegin);
		StaggerComp->OnStaggerRecovered.AddDynamic(this, &AKDEnemyBaseCharacter::OnStaggerRecovered);
	}
	if (ExecutionComp)
	{
		ExecutionComp->OnExecutionBegin.AddDynamic(this, &AKDEnemyBaseCharacter::OnExecutionBegin);

		// 처형 생존 리셋 — ExecutionComp.OnExecutionResolved -> StaggerComp.HandleExecutionResolved
		if (StaggerComp)
		{
			ExecutionComp->OnExecutionResolved.AddDynamic(StaggerComp, &UStaggerComponent::HandleExecutionResolved);
		}
	}
}

FVector AKDEnemyBaseCharacter::GetLockOnPoint_Implementation() const
{
	// 몸통 소켓 위치 — 소켓 X 시 캡슐 절반 높이
	if (const USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (!LockOnSocketName.IsNone() && MeshComp->DoesSocketExist(LockOnSocketName))
			return MeshComp->GetSocketLocation(LockOnSocketName);
	}
	const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
	return GetActorLocation() + FVector(0.f, 0.f, HalfHeight * 0.5f);
}

void AKDEnemyBaseCharacter::OnTargeted_Implementation(bool bIsTargeted)
{
	// 기능 : 락온 유무에 따른 상태 바 표시
	if (StateBarWidget)
	{
		StateBarWidget->SetVisibility(bIsTargeted);
	}
}

// AI 거리·공격셋 게터 — 값 = EnemyDefinition
float AKDEnemyBaseCharacter::GetSightRadius() const  { return EnemyDefinition ? EnemyDefinition->SightRadius : 1500.f; }
float AKDEnemyBaseCharacter::GetAttackRange() const  { return EnemyDefinition ? EnemyDefinition->AttackRange : 150.f; }
float AKDEnemyBaseCharacter::GetStandoffRange() const { return EnemyDefinition ? EnemyDefinition->StandoffRange : 0.f; }
float AKDEnemyBaseCharacter::GetPeripheralVisionAngle() const { return EnemyDefinition ? EnemyDefinition->PeripheralVisionAngle : 70.f; }
float AKDEnemyBaseCharacter::GetSightMemoryDuration() const { return EnemyDefinition ? EnemyDefinition->SightMemoryDuration : 4.f; }
float AKDEnemyBaseCharacter::GetPatrolRadius() const { return EnemyDefinition ? EnemyDefinition->PatrolRadius : 800.f; }
float AKDEnemyBaseCharacter::GetAttackSpeedMultiplier() const { return EnemyDefinition ? EnemyDefinition->AttackSpeedMultiplier : 1.0f; }
float AKDEnemyBaseCharacter::GetTelegraphSlowMultiplier() const { return EnemyDefinition ? EnemyDefinition->TelegraphSlowMultiplier : 1.0f; }
float AKDEnemyBaseCharacter::GetEngagementRange() const { return EnemyDefinition ? EnemyDefinition->EngagementRange : 700.0f; }
float AKDEnemyBaseCharacter::GetTurnRate() const { return EnemyDefinition ? EnemyDefinition->TurnRate : 360.0f; }

bool AKDEnemyBaseCharacter::IsStaggered() const { return StaggerComp && StaggerComp->IsStaggered(); }

void AKDEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 패트롤 기준점 = 스폰 위치
	HomeLocation = GetActorLocation();

	if (StateBarWidget)
	{
		if (UKDEnemyStateBarWidget* Bar = Cast<UKDEnemyStateBarWidget>(StateBarWidget->GetUserWidgetObject()))
		{
			Bar->SetTarget(this);
		}
	}
}

const TArray<FEnemyAttackEntry>& AKDEnemyBaseCharacter::GetAttackEntries() const
{
	static const TArray<FEnemyAttackEntry> Empty;
	return EnemyDefinition ? EnemyDefinition->Attacks : Empty;
}

void AKDEnemyBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 공격 토큰 반납 (멱등)
	ReturnAttackToken();

	Super::EndPlay(EndPlayReason);
}

void AKDEnemyBaseCharacter::ReturnAttackToken()
{
	if (UWorld* World = GetWorld())
	{
		if (UEncounterSubsystem* Encounter = World->GetSubsystem<UEncounterSubsystem>())
		{
			Encounter->ReturnToken(this);
		}
	}
}

void AKDEnemyBaseCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!bIsDead && Data.NewValue <= 0.0f)
	{
		HandleDeath();
	}
}

void AKDEnemyBaseCharacter::HandleDeath()
{
	bIsDead = true;

	// AbortForDeath 전에 캡처 — 처형 중 사망은 죽음 몽타주 스킵
	const bool bExecutionDeath = ExecutionComp && ExecutionComp->IsExecutionDeath();

	// 경직·처형 중 사망 — 양쪽 컴포넌트 상태 정리
	if (StaggerComp)
	{
		StaggerComp->AbortForDeath();
	}
	if (ExecutionComp)
	{
		ExecutionComp->AbortForDeath();
	}

	// 진행 중 GA 취소
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		// State.Dead — 데미지 GE·히트 큐 차단용
		AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::State_Dead);
	}

	// BT 정지
	if (AAIController* AICon = GetController<AAIController>())
	{
		AICon->StopMovement();

		// 포커스 해제 — StopLogic 은 focus 를 안 지움
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}

	// 공격 토큰 반납
	ReturnAttackToken();

	// 콜리전 + 무브먼트 정리
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	// 죽음 몽타주 재생 -> 블렌드아웃 시작에 랙돌 / 몽타주 X 시 즉시 랙돌
	// 처형 사망 = 몽타주 스킵 + 즉시 랙돌
	UAnimMontage* DeathMontage = EnemyDefinition ? EnemyDefinition->DeathMontage : nullptr;
	bool bPlayedDeathMontage = false;
	if (!bExecutionDeath && DeathMontage)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			{
				if (Anim->Montage_Play(DeathMontage) > 0.f)
				{
					bPlayedDeathMontage = true;

					// 블렌드아웃 시작에 랙돌 — End 는 포즈 팝 발생
					FOnMontageBlendingOutStarted BlendingOutDelegate;
					BlendingOutDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnDeathMontageEnded);
					Anim->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DeathMontage);

					// 백스톱 타이머 — 델리게이트 미도달 대비. EnterRagdoll 은 멱등
					FTimerHandle BackstopTimer;
					GetWorldTimerManager().SetTimer(BackstopTimer, this,
						&AKDEnemyBaseCharacter::EnterRagdoll, DeathMontage->GetPlayLength() + 0.5f, false);
				}
			}
		}
	}
	if (!bPlayedDeathMontage)
	{
		EnterRagdoll();
	}

	// BP 사망 반응 훅 — 디졸브 | 디스폰 타이머 | SFX
	OnDeath.Broadcast();
}

void AKDEnemyBaseCharacter::OnStaggerBegin()
{
	// brain 일시정지 + 이동 정지 — 재개 = OnStaggerRecovered
	if (AAIController* AICon = GetController<AAIController>())
	{
		AICon->StopMovement();

		// 포커스 해제 — PauseLogic 은 focus 를 안 지움
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("Staggered"));
		}
	}

	// desired-rotation 차단 — perception 자극이 SetCombatFacing 을 되살리는 경로 차단
	// 복구 = ResumeBrainFromStagger
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = false;        // 경직 중 회전 동결
	}

	// 경직 중 공격 X -> 토큰 양보
	ReturnAttackToken();

	// 처형·사망 몽타주 재생 시 자연 인터럽트
	if (UAnimMontage* StaggerMontage = EnemyDefinition ? EnemyDefinition->StaggerMontage : nullptr)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
			{
				Anim->Montage_Play(StaggerMontage);
			}
		}
	}
}

void AKDEnemyBaseCharacter::OnStaggerRecovered()
{
	// 사망 경로 = StopLogic("Dead") — brain resume 이 덮지 X
	if (bIsDead) { return; }

	UAnimMontage* StaggerMontage = EnemyDefinition ? EnemyDefinition->StaggerMontage : nullptr;
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;

	// 타임아웃 회복 — 경직 몽타주 루프 중 -> End 섹션 점프
	// 처형 생존 — AM_Execute 가 기상 포함 -> brain 만 재개
	const bool bStaggerLooping = StaggerMontage && Anim && Anim->Montage_IsPlaying(StaggerMontage);
	static const FName EndSection(TEXT("End"));
	if (bStaggerLooping && StaggerMontage->IsValidSectionName(EndSection))
	{
		Anim->Montage_JumpToSection(EndSection, StaggerMontage);
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnStaggerMontageEnded);
		Anim->Montage_SetEndDelegate(EndDelegate, StaggerMontage);
		return; // brain 복귀 = OnStaggerMontageEnded
	}

	// 처형 생존 또는 End 섹션 X -> brain 만 재개
	ResumeBrainFromStagger();
	if (bStaggerLooping) { Anim->Montage_Stop(0.25f, StaggerMontage); }
}

void AKDEnemyBaseCharacter::OnStaggerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// bInterrupted = 재경직·사망이 몽타주 점유 -> no-op
	if (bInterrupted || bIsDead) { return; }
	ResumeBrainFromStagger();
}

void AKDEnemyBaseCharacter::ResumeBrainFromStagger()
{
	// 경직 진입 시 끈 desired-rotation 복구
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = true;         // 경직 진입 시 끈 회전 복구
	}

	if (AAIController* AICon = GetController<AAIController>())
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("Staggered"));
		}
	}
}

void AKDEnemyBaseCharacter::ResumeBrainFromKnockback()
{
	// 기능 : 넉백 구간 종료 brain 재개
	// 사망·경직이 먼저 잡았으면 그쪽 정지 유지 — ResumeLogic 은 이유별 카운트 X
	if (bIsDead || (StaggerComp && StaggerComp->IsStaggered())) { return; }

	if (AAIController* AICon = GetController<AAIController>())
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("Knockback"));
		}
	}
}

void AKDEnemyBaseCharacter::OnHitReceived(const FGameplayEventData* Payload)
{
	// Dead·빈 페이로드 가드
	if (bIsDead || !Payload)
	{
		return;
	}

	ReportHitToPerception(Payload);

	const bool bStaggered = IsStaggered();

	// 경직 중 처형 히트 = 연출 X — 피니셔 연출이 따로 재생
	const bool bExecutionHit = bStaggered && ExecutionComp && ExecutionComp->IsExecutionTrigger(Payload->InstigatorTags);
	if (!bExecutionHit)
	{
		PlayHitFeedback(Payload);
	}

	// 경직 중 = Poise 차감·넉백 X
	if (bStaggered)
	{
		return;
	}

	// Poise 차감이 경직을 유발하면 넉백 X
	if (ApplyPoiseDamage(Payload))
	{
		return;
	}

	ApplyKnockback(Payload);
}

void AKDEnemyBaseCharacter::ReportHitToPerception(const FGameplayEventData* Payload)
{
	// 기능 : 피격을 AI 인지 자극으로 보고 — 시야 밖 공격도 전투 진입·타겟 기억 갱신
	// FGameplayEventData.Instigator = const -> cast
	if (!IsValid(Payload->Instigator))
	{
		return;
	}

	UAISense_Damage::ReportDamageEvent(GetWorld(), this, const_cast<AActor*>(Payload->Instigator.Get()),
		0.f, Payload->Instigator->GetActorLocation(), GetActorLocation());
}

void AKDEnemyBaseCharacter::PlayHitFeedback(const FGameplayEventData* Payload)
{
	// 기능 : 피격 연출 — 뼈 흔들림 + 타격 큐
	if (HitFeedback)
	{
		HitFeedback->TriggerBoneShake();
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_HitImpact_Light, Payload->ContextHandle);
	}
}

bool AKDEnemyBaseCharacter::ApplyPoiseDamage(const FGameplayEventData* Payload)
{
	// 기능 : 공격 태그별 Poise 차감 — 반환 = 이 차감으로 경직 진입 유무
	if (!IsValid(AbilitySystemComponent) || !EnemyDefinition || EnemyDefinition->PoiseDamageByAttack.Num() == 0)
	{
		return false;
	}

	// 태그가 여러 개 맞으면 합산
	float PoiseDamage = 0.f;
	for (const TPair<FGameplayTag, float>& Pair : EnemyDefinition->PoiseDamageByAttack)
	{
		if (Payload->InstigatorTags.HasTag(Pair.Key))
		{
			PoiseDamage += Pair.Value;
		}
	}

	if (PoiseDamage <= 0.f)
	{
		return false;
	}

	// 0 도달 시 StaggerComp.OnPoiseChanged 가 이 줄 안에서 BeginStagger 호출
	const float Cur = AbilitySystemComponent->GetNumericAttribute(UAS_CharacterBase::GetPoiseAttribute());
	AbilitySystemComponent->SetNumericAttributeBase(
		UAS_CharacterBase::GetPoiseAttribute(), FMath::Max(Cur - PoiseDamage, 0.f));

	return IsStaggered();
}

void AKDEnemyBaseCharacter::ApplyKnockback(const FGameplayEventData* Payload)
{
	// 기능 : 밀림 + 밀리는 동안 brain 정지
	// 경로추종 정지 — StopMovement 는 현재 요청만 취소, BT 가 다음 틱에 재요청
	// 넉백 구간만 brain 정지 — 재개 = ResumeBrainFromKnockback
	if (AAIController* AICon = GetController<AAIController>())
	{
		AICon->StopMovement();

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("Knockback"));
			GetWorldTimerManager().SetTimer(KnockbackBrainTimer, this,
				&AKDEnemyBaseCharacter::ResumeBrainFromKnockback, KnockbackBrainPause, false);
		}
	}

	// 넉백 세기 = 적 DA 기준값 x 공격 배수
	const float KnockbackMult = (Payload->EventMagnitude > 0.f) ? Payload->EventMagnitude : 1.f;
	const float KnockbackStrength = (EnemyDefinition ? EnemyDefinition->KnockbackStrength : 0.f) * KnockbackMult;
	FVector Dir = FVector::ZeroVector;
	if (KnockbackStrength > 0.f && IsValid(Payload->Instigator))
	{
		// 공격자 반대 방향 (수평) — ImpactNormal 은 캡슐 접선이라 미사용
		Dir = (GetActorLocation() - Payload->Instigator->GetActorLocation()).GetSafeNormal2D();

		// 폴백 = 공격자 정면 — 겹친 액터의 위치 델타 0
		if (Dir.IsNearlyZero())
		{
			Dir = Payload->Instigator->GetActorForwardVector().GetSafeNormal2D();
		}
	}

	if (!Dir.IsNearlyZero())
	{
		LaunchCharacter(Dir * KnockbackStrength, true, false);

#if !UE_BUILD_SHIPPING
		// 개발용 넉백 표시 — 공격 태그 / 배수 / 속도 / 0.1초 뒤 이동 거리 + 남은 속도
		if (CVarShowKnock.GetValueOnGameThread() > 0)
		{
			FString SrcTag = Payload->InstigatorTags.IsEmpty()
				? TEXT("-") : Payload->InstigatorTags.First().ToString();
			int32 DotIdx = INDEX_NONE;
			if (SrcTag.FindLastChar(TEXT('.'), DotIdx)) { SrcTag = SrcTag.RightChop(DotIdx + 1); }

			const FVector KnockStart = GetActorLocation();
			const float DbgMult = KnockbackMult;
			const float DbgSpeed = KnockbackStrength;
			FTimerHandle DbgKnockTimer;
			GetWorldTimerManager().SetTimer(DbgKnockTimer, FTimerDelegate::CreateWeakLambda(this,
				[this, SrcTag, DbgMult, DbgSpeed, KnockStart]()
				{
					const float Moved = FVector::Dist2D(GetActorLocation(), KnockStart);
					const float NowSpeed = GetVelocity().Size2D();
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
							FString::Printf(TEXT("Knock  %-12s x%.2f   speed %.0f   ->  %.0f cm   (남은속도 %.0f)"),
								*SrcTag, DbgMult, DbgSpeed, Moved, NowSpeed));
					}
				}), 0.1f, false);
		}
#endif
	}
	else if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// 넉백 0 또는 방향 X — 잔여 속도만 제거
		Move->StopMovementImmediately();
	}
}

void AKDEnemyBaseCharacter::OnExecutionBegin()
{
	// 피니셔 몽타주 재생 — 종료 델리게이트에서 FinishExecution
	if (!ExecutionComp)
	{
		return;
	}

	// 데스블로 = 죽음 피니셔 / 생존 = 다운 -> 기상. 판정 = ExecutionComp
	UAnimMontage* Montage = ExecutionComp->GetExecutionMontage();
	if (!Montage)
	{
		return;
	}
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			const float Len = Anim->Montage_Play(Montage);
			if (Len > 0.f)
			{
				if (ExecutionComp->IsDeathblow())
				{
					// 데스블로 — 블렌드아웃 시작에 결판
					FOnMontageBlendingOutStarted BlendingOutDelegate;
					BlendingOutDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnExecutionMontageEnded);
					Anim->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
				}
				else
				{
					// 생존 처형 — 기상 완료 후 brain 복귀
					FOnMontageEnded EndDelegate;
					EndDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnExecutionMontageEnded);
					Anim->Montage_SetEndDelegate(EndDelegate, Montage);
				}
			}
		}
	}
}

void AKDEnemyBaseCharacter::OnExecutionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// bInterrupted 무관 항상 결판 — Invulnerable + 어빌리티 취소 상태
	// FinishExecution = 이중 호출 가드 내장
	if (ExecutionComp)
	{
		ExecutionComp->FinishExecution();
	}
}

void AKDEnemyBaseCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 블렌드아웃 시작 -> 죽음 포즈 그대로 랙돌 인계
	EnterRagdoll();
}

void AKDEnemyBaseCharacter::EnterRagdoll()
{
	// 멱등 — 중복 호출 시 1회만 전환
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || MeshComp->IsSimulatingPhysics()) { return; }

	// Ragdoll 프로파일 ObjectType = PhysicsBody (ECC_Pawn X) — 무기 트레이스가 시체 무시
	// 꺾임 발생 시 확인 대상 = PhysicsAsset | 콜리전
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetSimulatePhysics(true);
}
