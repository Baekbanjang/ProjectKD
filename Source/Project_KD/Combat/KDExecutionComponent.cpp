// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/KDExecutionComponent.h"
#include "Combat/KDExecutionProfile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/KDCharacterAttributeSet.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Actor.h"
#include "KDGameplayTags.h"
#include "Components/CapsuleComponent.h"

UKDExecutionComponent::UKDExecutionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 처형 트리거 기본값 = 강공. BP에서 추가/변경 (적별 처형 조건 데이터 주도, OCP).
	ExecutionTriggerTags.AddTag(GameplayTags::Ability_Player_Execution);
}

void UKDExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	// IAbilitySystemInterface 경유 (concrete Pawn 캐스팅 금지).
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!OwnerASC){	return;	}
	ASC = OwnerASC;

	// OnWeaponHit이 보내는 제네릭 히트 이벤트 수신 — 경직 중 강공이면 처형.
	OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(GameplayTags::Event_Combat_Hit)
		.AddUObject(this, &UKDExecutionComponent::OnHitReceived);
}

void UKDExecutionComponent::OnHitReceived(const FGameplayEventData* Payload)
{
	// Dead or mid-execution → ignore: the execution cinematic locks the body in place.
	if (bIsBeingExecuted || !Payload || !ASC.IsValid()) { return; }
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Dead)) { return; }

	// 경직 중 처형 트리거 태그 피격 — Poise/넉백은 Pawn OnHitReceived가 경직 중 early-return하므로 중복 없음.
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Staggered)
		&& Payload->InstigatorTags.HasAny(ExecutionTriggerTags))
	{
		ExecutionInstigator = const_cast<AActor*>(Payload->Instigator.Get());
		HandleExecution();
	}
}

bool UKDExecutionComponent::IsExecutionTrigger(const FGameplayTagContainer& InstigatorTags) const
{
	return !ExecutionTriggerTags.IsEmpty() && InstigatorTags.HasAny(ExecutionTriggerTags);
}

UAnimMontage* UKDExecutionComponent::GetExecutionMontage() const
{
	if (!ExecutionProfile) { return nullptr; }
	// 데스블로면 죽음 피니셔, 미지정이면 기본(생존) 몽타주로 폴백. 생존 처형은 항상 기본 몽타주.
	if (bDeathblow && ExecutionProfile->DeathblowMontage) { return ExecutionProfile->DeathblowMontage; }
	return ExecutionProfile->Montage;
}

void UKDExecutionComponent::HandleExecution()
{
	bIsBeingExecuted = true;

	// 데스블로(치명) 판정 — 시작 시점에 결정해야 모션을 맞게 고름(데미지는 몽타주 끝에 적용되므로).
	// 생존 불가(잡몹)면 항상 데스블로. 생존형(엘리트)은 Health가 임계 이하일 때만 치명, 아니면 생존(다운→기상).
	bDeathblow = true;
	if (ExecutionProfile && ExecutionProfile->bSurvivable)
	{
		const float Health = ASC.IsValid() ? ASC->GetNumericAttribute(UKDCharacterAttributeSet::GetHealthAttribute()) : 0.f;
		bDeathblow = Health <= ExecutionProfile->DeathblowHealthThreshold;
	}

	float SafetyTime = ExecutionProfile ? ExecutionProfile->Duration : 1.5f;

	if (UAbilitySystemComponent* OwnerASC = ASC.Get())
	{
		// Shield the cinematic from other incoming damage (GE_Damage_Physical rejects Invulnerable).
		OwnerASC->AddLooseGameplayTag(GameplayTags::State_Combat_Invulnerable);

		if (UCapsuleComponent* Cap = GetOwner()->FindComponentByClass<UCapsuleComponent>())
		{
			Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
		// VFX/SFX — 큐 태그 등록 전까지 no-op. 모션은 Pawn이 OnExecutionBegin에서 재생.
		if (ExecutionProfile && ExecutionProfile->CueTag.IsValid())
		{
			OwnerASC->ExecuteGameplayCue(ExecutionProfile->CueTag, OwnerASC->MakeEffectContext());
		}
	}

	// Pawn이 OnExecutionBegin에서 몽타주 재생 후 FinishExecution 콜백 — 안전망 타이머는 그게 안 올 때만 발화.
	// 데스블로/생존 몽타주가 다르므로 GetExecutionMontage()로 길이 산정.
	if (UAnimMontage* PlayMontage = GetExecutionMontage())
	{
		SafetyTime = PlayMontage->GetPlayLength() + 0.5f;
	}

	if (ExecutionInstigator && GetOwner())
	{
		FVector ToInstigator = ExecutionInstigator->GetActorLocation() - GetOwner()->GetActorLocation();
		ToInstigator.Z = 0.f;
		if (!ToInstigator.IsNearlyZero())
		{
			GetOwner()->SetActorRotation(ToInstigator.Rotation());
		}
	}
	
	OnExecutionBegin.Broadcast();

	if (ExecutionInstigator)
	{
		FGameplayEventData Data;
		Data.Instigator = GetOwner();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			ExecutionInstigator, GameplayTags::Event_Combat_ExecutionStarted, Data);
	}

	// Safety net — fires only if the montage-end path never calls FinishExecution.
	GetWorld()->GetTimerManager().SetTimer(ExecutionTimer, this,
		&UKDExecutionComponent::FinishExecution, SafetyTime, false);
}

void UKDExecutionComponent::FinishExecution()
{
	// Guard double-fire (Pawn montage-end call + safety timer).
	if (!bIsBeingExecuted) { return; }
	bIsBeingExecuted = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionTimer);
	}

	UAbilitySystemComponent* OwnerASC = ASC.Get();
	if (!OwnerASC) { return; }

	// Lift the cinematic shield, then land the execution damage (Profile-tuned flat magnitude).
	OwnerASC->RemoveLooseGameplayTag(GameplayTags::State_Combat_Invulnerable);

	// 데스블로 = GE 수치 의존 없이 코드로 Health 0 확정(데미지 튜닝 부족으로 살아 AI 복귀하는 깨진 상태 방지).
	// SetNumericAttributeBase → OnHealthChanged 동기 발화 → HandleDeath 즉시 실행.
	// IsExecutionDeath()=true라 죽음 몽타주 스킵(처형 모션이 곧 죽음 연출). 래치는 AbortForDeath가 해제.
	if (bDeathblow)
	{
		bResolvingExecution = true;
		OwnerASC->SetNumericAttributeBase(UKDCharacterAttributeSet::GetHealthAttribute(), 0.f);
	}
	// 생존 처형(엘리트, Health>임계) = 칩 데미지만(비치명). 미지정이면 데미지 없음(HP는 일반 공격으로만).
	else if (ExecutionProfile && ExecutionProfile->SurviveDamageEffectClass)
	{
		FGameplayEffectContextHandle Ctx = OwnerASC->MakeEffectContext();
		Ctx.AddSourceObject(GetOwner());
		const FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(ExecutionProfile->SurviveDamageEffectClass, 1.f, Ctx);
		if (Spec.IsValid())
		{
			bResolvingExecution = true; // 칩이 만에 하나 치명타여도 처형사망으로 처리(죽음 몽타주 스킵).
			OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}

	// State.Dead 태그/델리게이트 타이밍 의존 없이 Health 값으로 직접 판정(동기 갱신 보장).
	// 생존 → 플래그 해제(이후 일반 사망은 죽음 몽타주 정상). 사망 → 래치 유지(지연 HandleDeath가 소비).
	const float Health = OwnerASC->GetNumericAttribute(UKDCharacterAttributeSet::GetHealthAttribute());
	const bool bSurvived = Health > 0.f;
	if (bSurvived)
	{
		bResolvingExecution = false;
	}

	if (UCapsuleComponent* Cap = GetOwner()->FindComponentByClass<UCapsuleComponent>())
	{
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	
	OnExecutionResolved.Broadcast(bSurvived);
}

void UKDExecutionComponent::AbortForDeath()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionTimer);
	}

	const bool bWasExecuting = bIsBeingExecuted;
	bIsBeingExecuted = false;

	// 처형 해소 창 종료 — HandleDeath가 이미 bExecutionDeath를 캡처한 뒤 호출하므로 여기서 안전하게 해제.
	bResolvingExecution = false;

	// Only drop the shield we added in HandleExecution (loose tag is ref-counted — don't over-remove
	// an Invulnerable granted by some other source).
	if (bWasExecuting)
	{
		if (UAbilitySystemComponent* OwnerASC = ASC.Get())
		{
			OwnerASC->RemoveLooseGameplayTag(GameplayTags::State_Combat_Invulnerable);
		}
	}

	if (UCapsuleComponent* Cap = GetOwner()->FindComponentByClass<UCapsuleComponent>())
	{
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}
