// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "KDGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/AnimNotifies/ANS_MeleeTrace.h"
#include "Combat/Data/HitConfirmProfile.h"


void UGA_PlayerMeleeAttackBase::OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit)
{
	UAbilitySystemComponent* AttackerASC = GetAbilitySystemComponentFromActorInfo();
	if (!AttackerASC || !TargetASC) return;
	
	// i-frame 닷지/사망한 대상엔 타격감 큐 생략
	if (TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Invulnerable)
		|| TargetASC->HasMatchingGameplayTag(GameplayTags::State_Dead))
	{
		return;
	}
	
	// 큐가 ImpactPoint에 정확히 찍히도록 컨텍스트에 HitResult 동봉
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());
	Context.AddHitResult(Hit);
	
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;
	CueParams.RawMagnitude = HitConfirmMagnitude;
	CueParams.EffectContext = Context;
	CueParams.SourceObject = HitConfirmProfile;
	
	AttackerASC->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_PlayerHitConfirm, CueParams);

	// 히트스탑 윈도우 확보 - bIgnoreHitStop 확인 후 생략 가능
	const UANS_MeleeTrace* Window = GetActiveWindow();
	if (Window && Window->bIgnoreHitStop) { return; }
	
	// 정지 주체는 공격자 HitStopComponent
	FGameplayEventData StopEvent;
	StopEvent.EventMagnitude = AttackerHitStopDuration;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetAvatarActorFromActorInfo(), GameplayTags::Event_Combat_HitStop, StopEvent);
}

void UGA_PlayerMeleeAttackBase::OnActivated()
{
	ACharacter* Attacker = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Attacker) return;

	// 워프 컴포넌트 = ApproachWarpName 있을 때만
	UMotionWarpingComponent* Warp = ApproachWarpName.IsNone()
		? nullptr : Attacker->FindComponentByClass<UMotionWarpingComponent>();

	// 지난 공격 좌표 제거 — 타겟 없는 경로에서 옛 좌표로 끌려감 방지
	if (Warp) Warp->RemoveWarpTarget(ApproachWarpName);
	
	AActor* Target = FindAutoAimTarget(AutoAimRange, AutoAimConeAngle);
	if (!Target) return;
	
	// 락온 자동 조준 
	const FVector ToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	// 뒤쪽 135도 초과는 제외
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(Attacker->GetActorRotation().Yaw, ToTarget.Rotation().Yaw);
	Attacker->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));

	if (!Warp) return;
	USceneComponent* TargetRoot = Target->GetRootComponent();

	if (!TargetRoot) return;
	const float Dist = FVector::Dist2D(Target->GetActorLocation(), Attacker->GetActorLocation());

#if !UE_BUILD_SHIPPING
	// 개발용 접근 판정 표시 — 거리 + 어느 게이트에 걸렸는지
	if (GEngine)
	{
		const TCHAR* Why = (Dist > MaxApproachRange) ? TEXT("멂")
			: (Dist <= ApproachStopDistance ? TEXT("붙음") : TEXT("워프"));
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Approach %s   dist %.0f   (stop %.0f / max %.0f)"),
				Why, Dist, ApproachStopDistance, MaxApproachRange));
	}
#endif

	// 사거리 밖 = 제자리 / 코앞 = 몽타주 원래 이동량 유지
	if (Dist > MaxApproachRange || Dist <= ApproachStopDistance) return;
	// 타겟 추적 등록 
	// 오프셋 X = 적에서 나 쪽으로 ApproachStopDistance 만큼
	Warp->AddOrUpdateWarpTargetFromComponent(
		ApproachWarpName, TargetRoot, NAME_None, true,
		EWarpTargetLocationOffsetDirection::VectorFromTargetToOwner,
		FVector(ApproachStopDistance, 0.f, 0.f), FRotator::ZeroRotator
	);
}
