// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/KDKnockbackComponent.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


#if !UE_BUILD_SHIPPING
// 개발용 넉백 표시 스위치 — 콘솔 KD.ShowKnock 1
static TAutoConsoleVariable<int32> CVarShowKnock(
	TEXT("KD.ShowKnock"), 0,
	TEXT("넉백 거리 온스크린 표시 유무"), ECVF_Cheat);
#endif

static const FName KnockbackSourceName(TEXT("Knockback"));

UKDKnockbackComponent::UKDKnockbackComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
}

void UKDKnockbackComponent::ApplyKnockback(const FGameplayEventData& Payload, float BaseDistance)
{
	// 기능 : 공격자 반대 수평으로 지정 거리만큼 밀기
	ACharacter* OwnerChar = GetOwner<ACharacter>();
	UCharacterMovementComponent* Move = OwnerChar ? OwnerChar->GetCharacterMovement() : nullptr;
	if (!Move)
	{
		return;
	}

	// 적 AI 정지 요청
	OnKnockbackBegin.Broadcast();

	// 넉백 거리 = 적 DA 기준값 X 공격 배수
	const float Mult = (Payload.EventMagnitude > 0.f) ? Payload.EventMagnitude : 1.f;
	const float Distance = BaseDistance * Mult;
	FVector Dir = FVector::ZeroVector;
	if (Distance > 0.f && IsValid(Payload.Instigator))
	{
		// 공격자 반대 방향
		Dir = (OwnerChar->GetActorLocation() - Payload.Instigator->GetActorLocation()).GetSafeNormal2D();

		// 폴백 = 공격자 정면 - 겹친 액터의 위치 델타 0
		// IsNearlyZero = XYZ 전부 오차범위(1e-4) 이하 유무 — 부동소수점이라 == 0 비교 X
		if (Dir.IsNearlyZero())
		{
			Dir = Payload.Instigator->GetActorForwardVector().GetSafeNormal2D();
		}
	}
	if (Dir.IsNearlyZero())
	{
		// 밀림 0 또는 방향 X
		Move->StopMovementImmediately();
		return;
	}

	// 직전 밀림 제거 — 연타 시 두 소스가 겹치면 이동량 합산
	Move->RemoveRootMotionSource(KnockbackSourceName);
	const FVector Start = OwnerChar->GetActorLocation();
	TSharedPtr<FRootMotionSource_MoveToForce> Source = MakeShared<FRootMotionSource_MoveToForce>();
	Source->InstanceName = KnockbackSourceName;
	Source->AccumulateMode = ERootMotionAccumulateMode::Override;
	Source->Priority = 5;
	Source->Duration = KnockbackDuration;
	Source->StartLocation = Start;
	Source->TargetLocation = Start + Dir * Distance;
	Source->bRestrictSpeedToExpected = true;
	Move->ApplyRootMotionSource(Source);

#if !UE_BUILD_SHIPPING
	// 개발용 넉백 표시 — 공격 태그 / 배수 / 목표 거리 / 실제 거리
	if (CVarShowKnock.GetValueOnGameThread() > 0)
	{
		FString SrcTag = Payload.InstigatorTags.IsEmpty()
			? TEXT("-") : Payload.InstigatorTags.First().ToString();
		int32 DotIdx = INDEX_NONE;
		if (SrcTag.FindLastChar(TEXT('.'), DotIdx)) { SrcTag = SrcTag.RightChop(DotIdx + 1); }
		FTimerHandle DbgTimer;
		GetWorld()->GetTimerManager().SetTimer(DbgTimer, FTimerDelegate::CreateWeakLambda(this,
			[this, SrcTag, Mult, Distance, Start]()
			{
				const AActor* Owner = GetOwner();
				const float Moved = Owner ? FVector::Dist2D(Owner->GetActorLocation(), Start) : 0.f;
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
						FString::Printf(TEXT("Knock  %-12s x%.2f   목표 %.0f cm   ->  실제 %.0f cm"),
							*SrcTag, Mult, Distance, Moved));
				}
			}), KnockbackDuration + 0.05f, false);
	}
#endif
}




