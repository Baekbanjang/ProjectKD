// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/KDStaggerComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "AbilitySystem/Effects/GE_Stagger.h"
#include "KDGameplayTags.h"

UKDStaggerComponent::UKDStaggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// F10 GE 기본값 = C++ base (테스트 즉시 동작). 튜닝 필요 시 BP child로 교체.
	StaggerEffectClass = UGE_Stagger::StaticClass();
}

void UKDStaggerComponent::BeginPlay()
{
	Super::BeginPlay();

	// IAbilitySystemInterface 경유 (concrete Pawn 캐스팅 금지).
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!OwnerASC) { return; }
	ASC = OwnerASC;

	// AttributeSet는 데이터만 — 구독 및 경직 판정은 컴포넌트 여기서.
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UAS_CharacterBase::GetPoiseAttribute())
		.AddUObject(this, &UKDStaggerComponent::OnPoiseChanged);
}

bool UKDStaggerComponent::IsStaggered() const
{
	return ASC.IsValid() && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Staggered);
}

void UKDStaggerComponent::OnPoiseChanged(const FOnAttributeChangeData& Data)
{
	// Poise는 회복 없이 하강만 — 0 도달 시 경직 진입.
	if (!IsStaggered() && Data.NewValue <= 0.0f)
	{
		BeginStagger();
	}
}

void UKDStaggerComponent::BeginStagger()
{
	if (UAbilitySystemComponent* OwnerASC = ASC.Get())
	{
		// Interrupt whatever the enemy is mid-action on (current attack montage GA).
		OwnerASC->CancelAllAbilities();

		// State.Combat.Staggered 부여 → 적 GA가 ActivationBlockedTags로 자가 차단.
		if (StaggerEffectClass)
		{
			FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
			Context.AddSourceObject(GetOwner());
			const FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(StaggerEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				StaggerEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}
	}

	// StaggerDuration 후 자동 회복. 처형 중 타임아웃이 떨어져도 피니셔 도중 un-stagger되면 안 되므로
	// 가드 경유(OnStaggerTimeout) — 처형 resolved 콜백이 회복 주도.
	GetWorld()->GetTimerManager().SetTimer(StaggerTimeoutTimer, this,
		&UKDStaggerComponent::OnStaggerTimeout, StaggerDuration, false);

	// Pawn이 brain pause + movement freeze를 여기서 받음.
	OnStaggerBegin.Broadcast();
}

void UKDStaggerComponent::OnStaggerTimeout()
{
	// 처형 진행 중(State.Combat.Invulnerable)이면 skip — 처형 resolved 콜백이 회복 주도.
	if (ASC.IsValid() && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Invulnerable)) { return; }
	RecoverFromStagger();
}

void UKDStaggerComponent::RecoverFromStagger()
{
	// 처형이 타이머보다 먼저 호출할 수 있으니 clear (나중 타임아웃 중복 발동 방지).
	GetWorld()->GetTimerManager().ClearTimer(StaggerTimeoutTimer);

	UAbilitySystemComponent* OwnerASC = ASC.Get();
	if (!OwnerASC) { return; }

	// State.Combat.Staggered 제거 → 적 GA 재활성 가능.
	if (StaggerEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(StaggerEffectHandle);
		StaggerEffectHandle.Invalidate();
	}

	// 경직 사이클 종료 시 Poise 풀 리셋 — 재경직하려면 다시 0까지 깎아야 함.
	const float MaxPoise = OwnerASC->GetNumericAttribute(UAS_CharacterBase::GetMaxPoiseAttribute());
	OwnerASC->SetNumericAttributeBase(UAS_CharacterBase::GetPoiseAttribute(), MaxPoise);

	// Pawn resumes the brain on this.
	OnStaggerRecovered.Broadcast();
}

void UKDStaggerComponent::HandleExecutionResolved(bool bSurvived)
{
	// 생존(엘리트/보스) → 기상 후 전투 복귀(Poise 풀 리셋 포함).
	// 사망 경로는 HandleDeath → AbortForDeath로 이미 처리됐으므로 non-survival은 no-op.
	if (bSurvived)
	{
		RecoverFromStagger();
	}
}

void UKDStaggerComponent::AbortForDeath()
{
	// Died mid-stagger → kill the pending recovery so it can't fire on a corpse.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaggerTimeoutTimer);
	}

	// Drop the stagger GE so State.Combat.Staggered doesn't linger on the corpse.
	if (UAbilitySystemComponent* OwnerASC = ASC.Get())
	{
		if (StaggerEffectHandle.IsValid())
		{
			OwnerASC->RemoveActiveGameplayEffect(StaggerEffectHandle);
			StaggerEffectHandle.Invalidate();
		}
	}
}
