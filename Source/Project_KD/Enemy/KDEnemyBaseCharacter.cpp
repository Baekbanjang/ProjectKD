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

AKDEnemyBaseCharacter::AKDEnemyBaseCharacter()
{
	// Enemy owns its ASC directly (player's lives on PlayerState).
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CharacterAttributes = CreateDefaultSubobject<UAS_CharacterBase>(TEXT("CharacterAttributes"));
	CombatAttributes = CreateDefaultSubobject<UAS_Combat>(TEXT("CombatAttributes"));

	// Minimal mode — AI ASC, no owning client needs full GE detail (only cues replicate).
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HitFeedback = CreateDefaultSubobject<UHitFeedbackComponent>(TEXT("HitFeedback"));

	// 경직/처형은 전용 컴포넌트로 분리 — 각자 BeginPlay에서 ASC 캐시 + 이벤트 구독.
	StaggerComp = CreateDefaultSubobject<UStaggerComponent>(TEXT("StaggerComp"));
	ExecutionComp = CreateDefaultSubobject<UExecutionComponent>(TEXT("ExecutionComp"));

	// Auto-possess so a placed/spawned enemy gets its AIController without manual wiring.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AKDEnemyAIController::StaticClass();

	// 전투 facing은 controller desired rotation으로 — SetFocus/ClearFocus는 BTService_FindPlayer가 관리.
	// BP child에서 아래 회전 플래그 직접 수정 금지(런타임에 덮어써짐).
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = true;
		Move->bOrientRotationToMovement = false;
		Move->RotationRate = FRotator(0.f, 360.f, 0.f);     // 턴속도(도/초) — PossessedBy에서 DataAsset 값으로 덮음
	}
}

void AKDEnemyBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!IsValid(AbilitySystemComponent)) return;

	// Owner = Avatar = this Pawn (no PlayerState in the loop for enemies).
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Faction tag — the weapon-trace GA gates enemy-vs-enemy friendly fire on this (player ASC has none).
	AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::Team_Enemy);

	// 적별 스탯/StartupAbilities는 EnemyDefinition 소유. 미할당이면 기본값 유지 + 부여 스킵(크래시 방지).
	if (ensureMsgf(EnemyDefinition != nullptr, TEXT("EnemyDefinition unset on %s — using AttributeSet defaults"), *GetName()))
	{
		// 델리게이트 바인딩 전에 스탯 set — 순서 역전 시 init이 OnHealthChanged를 spurious 사망으로 깨운다.
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetMaxHealthAttribute(), EnemyDefinition->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetHealthAttribute(),    EnemyDefinition->MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetMaxPoiseAttribute(),  EnemyDefinition->MaxPoise);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_CharacterBase::GetPoiseAttribute(),     EnemyDefinition->MaxPoise);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_Combat::GetAttackPowerAttribute(),      EnemyDefinition->AttackPower);
		AbilitySystemComponent->SetNumericAttributeBase(UAS_Combat::GetDefenseAttribute(),          EnemyDefinition->Defense);

		// Single-player demo: grant on the authoritative side. Multiplayer would gate on HasAuthority().
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : EnemyDefinition->StartupAbilities)
		{
			if (!AbilityClass) continue;
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}

		// DataAsset 턴속도 적용 — 생성자에서 설정한 기본값(360)을 덮어씀.
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->RotationRate.Yaw = GetTurnRate();
		}
	}

	// Pawn은 Health→사망만 구독. Poise→stagger는 StaggerComponent, 처형은 ExecutionComponent가 각자 구독.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAS_CharacterBase::GetHealthAttribute())
		.AddUObject(this, &AKDEnemyBaseCharacter::OnHealthChanged);

	// Poise 차감 + 넉백은 Pawn 잔류. 처형 트리거는 ExecutionComponent가 별도 구독(ASC 다중 구독자 순서 미보장).
	AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(GameplayTags::Event_Combat_Hit)
		.AddUObject(this, &AKDEnemyBaseCharacter::OnHitReceived);

	// 컴포넌트 배선은 델리게이트로만.
	if (StaggerComp)
	{
		StaggerComp->OnStaggerBegin.AddDynamic(this, &AKDEnemyBaseCharacter::OnStaggerBegin);
		StaggerComp->OnStaggerRecovered.AddDynamic(this, &AKDEnemyBaseCharacter::OnStaggerRecovered);
	}
	if (ExecutionComp)
	{
		ExecutionComp->OnExecutionBegin.AddDynamic(this, &AKDEnemyBaseCharacter::OnExecutionBegin);

		// 처형 생존 리셋 — ExecutionComponent.OnExecutionResolved → StaggerComponent.HandleExecutionResolved.
		if (StaggerComp)
		{
			ExecutionComp->OnExecutionResolved.AddDynamic(StaggerComp, &UStaggerComponent::HandleExecutionResolved);
		}
	}
}

FVector AKDEnemyBaseCharacter::GetLockOnPoint_Implementation() const
{
	// 적 몸통 소켓으로 고정 없으면 캡슐 절반 높이
	if (const USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (!LockOnSocketName.IsNone() && MeshComp->DoesSocketExist(LockOnSocketName))
			return MeshComp->GetSocketLocation(LockOnSocketName);
	}
	const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
	return GetActorLocation() + FVector(0.f, 0.f, HalfHeight * 0.5f);
}

// AI 거리/공격셋 게터 — 값은 EnemyDefinition 소유. 미할당 시 안전 기본값(호출부 무변경 이음새).
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

	// 패트롤 기준점 = 스폰 위치. 이후 이동해도 홈으로 회귀하는 순찰 중심.
	HomeLocation = GetActorLocation();
}

const TArray<FEnemyAttackEntry>& AKDEnemyBaseCharacter::GetAttackEntries() const
{
	static const TArray<FEnemyAttackEntry> Empty;
	return EnemyDefinition ? EnemyDefinition->Attacks : Empty;
}

void AKDEnemyBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 소멸/레벨 종료 시 보유 토큰 반납(멱등 — 안 들고 있어도 무해).
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

	// AbortForDeath가 상태를 지우기 전에 캡처 — 처형 중 사망은 처형 모션이 곧 죽음 연출이라 죽음 몽타주 스킵.
	const bool bExecutionDeath = ExecutionComp && ExecutionComp->IsExecutionDeath();

	// 스태거/처형 중 사망 — 양쪽 컴포넌트 상태(타이머·GE·시네마틱 쉴드) 정리해 시체에 누수 방지.
	if (StaggerComp)
	{
		StaggerComp->AbortForDeath();
	}
	if (ExecutionComp)
	{
		ExecutionComp->AbortForDeath();
	}

	// Interrupt any in-flight GA (attack montage) — brain stop below prevents re-activation.
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		// State.Dead — 이후 데미지 GE·히트 큐가 이 태그로 시체를 차단.
		AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::State_Dead);
	}

	// Stop the AI brain so the BehaviorTree quits ticking (chase/attack tasks keep running otherwise).
	if (AAIController* AICon = GetController<AAIController>())
	{
		AICon->StopMovement();

		// 포커스 해제 — StopLogic은 BT만 멈추고 focus는 남아, 시체가 죽음 몽타주 중에도 회전함.
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}

	// 토큰 반납 — 죽은 적이 슬롯을 잡고 있으면 대기 적이 못 들어옴(despawn 타이머 동안에도).
	ReturnAttackToken();

	// Stop driving the pawn: clear collision + movement so the corpse doesn't block or slide.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	// 죽음 몽타주 재생 → 블렌드아웃 시작에 랙돌(하이브리드). 미지정/실패는 즉시 랙돌.
	// 처형 사망은 처형 피니셔가 곧 죽음 연출 — 몽타주 건너뛰고 바로 랙돌.
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

					// 블렌드아웃 "시작"에 랙돌 — End(완료 후)면 ABP가 idle 블렌드를 끼워 포즈 팝 발생.
					FOnMontageBlendingOutStarted BlendingOutDelegate;
					BlendingOutDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnDeathMontageEnded);
					Anim->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DeathMontage);

					// 백스톱 — AutoBlendOut off 등으로 델리게이트가 안 오면 동결 시체 방지. EnterRagdoll은 멱등.
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

	// BP reacts (dissolve, despawn timer, SFX) via this hook.
	OnDeath.Broadcast();
}

void AKDEnemyBaseCharacter::OnStaggerBegin()
{
	// brain 일시정지 + 이동 정지 — 경직 몽타주가 미끄러지지 않게. OnStaggerRecovered에서 재개.
	if (AAIController* AICon = GetController<AAIController>())
	{
		AICon->StopMovement();

		// 포커스 해제 — PauseLogic은 BT만 멈추고 focus는 남아 경직 몸이 계속 플레이어를 추적함.
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("Staggered"));
		}
	}

	// desired-rotation 차단 — ClearFocus/PauseLogic 후에도 perception 자극이 SetCombatFacing을 되살려
	// 경직 몸이 플레이어를 따라 도는 것을 막음. 회복 시 ResumeBrainFromStagger가 복구.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseControllerDesiredRotation = false;        // 경직 중 회전 동결
	}

	// 경직 중엔 공격 불가 → 토큰 양보(슬롯 비워 대기 적이 협공 이어가게).
	ReturnAttackToken();

	// 처형/사망 몽타주가 나중에 재생되면 자연 인터럽트됨.
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
	// Death path uses StopLogic("Dead") which a brain resume won't override — a corpse stays put.
	if (bIsDead) { return; }

	UAnimMontage* StaggerMontage = EnemyDefinition ? EnemyDefinition->StaggerMontage : nullptr;
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;

	// 타임아웃 회복 시 스태거 몽타주가 루프 중 → End 섹션으로 점프해 기상.
	// 처형 생존은 AM_Execute가 기상까지 자체 포함 → 여기서 스태거 몽타주를 틀면 이중 기상됨. brain만 재개.
	const bool bStaggerLooping = StaggerMontage && Anim && Anim->Montage_IsPlaying(StaggerMontage);
	static const FName EndSection(TEXT("End"));
	if (bStaggerLooping && StaggerMontage->IsValidSectionName(EndSection))
	{
		Anim->Montage_JumpToSection(EndSection, StaggerMontage);
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnStaggerMontageEnded);
		Anim->Montage_SetEndDelegate(EndDelegate, StaggerMontage);
		return; // brain은 OnStaggerMontageEnded에서 복귀.
	}

	// 처형 생존(스태거 몽타주 미재생) 또는 End 섹션 없는 단일 스태거 → brain만 재개.
	ResumeBrainFromStagger();
	if (bStaggerLooping) { Anim->Montage_Stop(0.25f, StaggerMontage); }
}

void AKDEnemyBaseCharacter::OnStaggerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// bInterrupted = 재경직/사망이 새 몽타주로 점유 → 그 권위가 brain을 관리하므로 no-op.
	if (bInterrupted || bIsDead) { return; }
	ResumeBrainFromStagger();
}

void AKDEnemyBaseCharacter::ResumeBrainFromStagger()
{
	// 경직 진입 시 끈 desired-rotation 복구 — 회복 시점엔 IsStaggered()=false라 SetCombatFacing이 정상 동작.
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

void AKDEnemyBaseCharacter::OnHitReceived(const FGameplayEventData* Payload)
{
	// Dead/빈 페이로드 가드. (staggered 처리는 아래 — 충격 피드백만 주고 poise·넉백은 스킵)
	if (bIsDead || !Payload)
	{
		return;
	}

	// 피격 = 인지 자극 — 등 뒤/시야 밖 공격에도 전투 진입·타겟 기억 갱신(시야와 같은 타겟 파이프 합류).
	// FGameplayEventData.Instigator는 const라 cast (ReportDamageEvent는 읽기만 함).
	if (IsValid(Payload->Instigator))
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, const_cast<AActor*>(Payload->Instigator.Get()),
			0.f, Payload->Instigator->GetActorLocation(), GetActorLocation());
	}

	const bool bStaggered = StaggerComp && StaggerComp->IsStaggered();

	// 경직 중 칩 피격도 shake + SFX로 반응. 처형 히트만 제외(처형 레인이 큐/몽타주 담당, 이중 연출 방지).
	const bool bExecutionHit = bStaggered && ExecutionComp && ExecutionComp->IsExecutionTrigger(Payload->InstigatorTags);
	if (!bExecutionHit)
	{
		if (HitFeedback)
		{
			HitFeedback->TriggerBoneShake();
		}
		if (IsValid(AbilitySystemComponent))
		{
			AbilitySystemComponent->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_HitImpact_Light, Payload->ContextHandle);
		}
	}

	// A staggered body takes no poise drain / knockback (frozen — execution or chip already gave feedback).
	if (bStaggered)
	{
		return;
	}

	// Poise 차감 — 공격 태그별 데미지. 플레이어 공격만 등록(적끼리 friendly fire 무시).
	// 0 도달 시 StaggerComponent.BeginStagger.
	if (IsValid(AbilitySystemComponent) && EnemyDefinition && EnemyDefinition->PoiseDamageByAttack.Num() > 0)
	{
		float PoiseDamage = 0.f;
		for (const TPair<FGameplayTag, float>& Pair : EnemyDefinition->PoiseDamageByAttack)
		{
			if (Payload->InstigatorTags.HasTag(Pair.Key))
			{
				PoiseDamage += Pair.Value;
			}
		}
		if (PoiseDamage > 0.f)
		{
			const float Cur = AbilitySystemComponent->GetNumericAttribute(UAS_CharacterBase::GetPoiseAttribute());
			AbilitySystemComponent->SetNumericAttributeBase(
				UAS_CharacterBase::GetPoiseAttribute(), FMath::Max(Cur - PoiseDamage, 0.f));
		}
	}

	// 히트 넉백 — 위 poise 차감이 경직을 유발했으면 스킵(BeginStagger가 이미 동기 실행됨).
	if (!(StaggerComp && StaggerComp->IsStaggered()))
	{
		// 경로추종 먼저 끊음 — 안 끊으면 다음 틱 MoveTo가 넉백을 도로 밀어 문워크 발생.
		if (AAIController* AICon = GetController<AAIController>())
		{
			AICon->StopMovement();
		}

		// 넉백 세기는 EnemyDefinition 소유 — 미할당이면 0(넉백 없음).
		const float KnockbackStrength = EnemyDefinition ? EnemyDefinition->KnockbackStrength : 0.f;
		FVector Dir = FVector::ZeroVector;
		if (KnockbackStrength > 0.f && IsValid(Payload->Instigator))
		{
			// 공격자 반대 방향(수평). ImpactNormal은 캡슐 접선이라 측면 히트가 옆으로 날아가서 교체.
			Dir = (GetActorLocation() - Payload->Instigator->GetActorLocation()).GetSafeNormal2D();

			// Fallback: attacker facing — stacked/overlapping actors give a degenerate position delta.
			if (Dir.IsNearlyZero())
			{
				Dir = Payload->Instigator->GetActorForwardVector().GetSafeNormal2D();
			}
		}

		if (!Dir.IsNearlyZero())
		{
			LaunchCharacter(Dir * KnockbackStrength, true, false);
		}
		else if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			// 넉백 0(또는 방향 degenerate) — 제자리 움찔이므로 잔여 속도만 제거.
			Move->StopMovementImmediately();
		}
	}
}

void AKDEnemyBaseCharacter::OnExecutionBegin()
{
	// 피니셔 몽타주 재생 — 종료 델리게이트에서 FinishExecution. 안전망 타이머는 ExecutionComponent가 소유.
	if (!ExecutionComp)
	{
		return;
	}

	// 데스블로(치명)면 죽음 피니셔, 생존이면 다운→기상 — ExecutionComponent가 판정해 알려줌.
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
					// 데스블로 — 블렌드아웃 "시작"에 결판(End까지 기다리면 ABP idle 블렌드가 끼어 포즈 팝).
					FOnMontageBlendingOutStarted BlendingOutDelegate;
					BlendingOutDelegate.BindUObject(this, &AKDEnemyBaseCharacter::OnExecutionMontageEnded);
					Anim->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
				}
				else
				{
					// 생존 처형 — 기상까지 다 끝난 진짜 끝에서 brain 복귀(블렌드아웃 동안은 아직 기상 중).
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
	// bInterrupted 무관 항상 결판 — Invulnerable + 어빌리티 취소 상태라 진짜 인터럽트는 없음.
	// FinishExecution은 이중 호출 가드 내장(안전망 타이머 중복 무해).
	if (ExecutionComp)
	{
		ExecutionComp->FinishExecution();
	}
}

void AKDEnemyBaseCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 죽음 연출 블렌드아웃 시작 → 죽음 포즈 그대로 랙돌 인계(지형 안착, 하이브리드).
	EnterRagdoll();
}

void AKDEnemyBaseCharacter::EnterRagdoll()
{
	// 멱등 — 블렌드아웃 델리게이트/백스톱 타이머/처형 경로가 중복 호출해도 1회만 전환.
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || MeshComp->IsSimulatingPhysics()) { return; }

	// Ragdoll 프로파일 ObjectType은 PhysicsBody(ECC_Pawn 아님)라 무기 트레이스가 시체를 무시 —
	// State.Dead GE 가드의 물리적 보완. 꺾임 발생 시 PhysicsAsset/콜리전을 볼 것(코드 아님, 2026-06-11 확정).
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetSimulatePhysics(true);
}
