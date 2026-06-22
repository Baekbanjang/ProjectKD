// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatStateComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Engine/OverlapResult.h"

UCombatStateComponent::UCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		ScanTimerHandle, this, &UCombatStateComponent::ScanForEnemies, ScanInterval, true);

	
	RegisterAttackingTagListener();
}

void UCombatStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 델리게이트 Attacking 종료
	if (AttackingTagHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_Attacking, EGameplayTagEventType::NewOrRemoved).Remove(AttackingTagHandle);
		}
		AttackingTagHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UCombatStateComponent::ScanForEnemies()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;

	const bool bHasInCombat = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InCombat);
	const bool bAttacking = ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking);
	
	if (HasEnemyInRange() || bAttacking)
	{
		// 적 있음 또는 공격 -> 해제 대기 취소 + 전투 태그 부여(중복 방지).
		GetWorld()->GetTimerManager().ClearTimer(ExitTimerHandle);
		if (!bHasInCombat)
		{
			ASC->AddLooseGameplayTag(GameplayTags::State_Combat_InCombat);
		}
	}
	else if (bHasInCombat && !GetWorld()->GetTimerManager().IsTimerActive(ExitTimerHandle))
	{
		// 적 없음 -> 바로 안 끄고 CombatExitDelay 뒤 해제
		TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
		FTimerDelegate ExitDel = FTimerDelegate::CreateLambda([WeakASC]()
		{
			if (WeakASC.IsValid())
			{
				WeakASC->RemoveLooseGameplayTag(GameplayTags::State_Combat_InCombat);
			}
		});
		GetWorld()->GetTimerManager().SetTimer(ExitTimerHandle, ExitDel, CombatExitDelay, false);
	}
}

bool UCombatStateComponent::HasEnemyInRange() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	const FVector OwnerLoc = Owner->GetActorLocation();

	// 반경 내 Pawn 후보 수집 
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OwnerLoc,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(DetectionRadius),
		Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Owner) continue;

		// Team.Enemy 보유 + 살아있음(State.Dead X)
		UAbilitySystemComponent* CandASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!CandASC) continue;
		if (!CandASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)) continue;
		if (CandASC->HasMatchingGameplayTag(GameplayTags::State_Dead)) continue;

		return true; // 하나라도 있으면 전투
	}

	return false;
}

// 공격 태그 델리게이트 등록
void UCombatStateComponent::RegisterAttackingTagListener()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UCombatStateComponent::RegisterAttackingTagListener);
		}
		return;
	}

	AttackingTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_Attacking, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UCombatStateComponent::OnAttackingTagChanged);
}

// 공격 시작 시 즉시 전투
void UCombatStateComponent::OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;
	
	GetWorld()->GetTimerManager().ClearTimer(ExitTimerHandle);
	if (!ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InCombat))
	{
		ASC->AddLooseGameplayTag(GameplayTags::State_Combat_InCombat);
	}
}

