// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/KDGameplayAbility_AreaBlast.h"
#include "KDGameplayTags.h"
#include "Combat/Data/KDHitConfirmProfile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/KDCombatAttributeSet.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"

UKDGameplayAbility_AreaBlast::UKDGameplayAbility_AreaBlast()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;
	
	// 노티 이벤트로 자동 활성 - 트리거
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = GameplayTags::Event_Montage_AreaBlast;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
	
	// 전방위 원기둥 
	AreaFilter.ShapeType = EKDTargetShapeType::Cylinder;
	AreaFilter.Basis = EKDTargetBasisType::CharacterForward;
	AreaFilter.Radius = 300.f;
	AreaFilter.Height = 300.f;
	AreaFilter.HeightOffset = -100.f;
}

void UKDGameplayAbility_AreaBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 기능 : 범위 판정 1회 후 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Avatar) || !DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 맞은 적 체크
	TArray<FHitResult> Hits;
	GatherTargets(Hits);
	bool bAnyHit = false;
	for (const FHitResult& Hit : Hits)
	{
		if (ApplyHit(Hit)) bAnyHit = true;
	}
	
	// 히트스톱 1회
	if (bAnyHit && AttackerHitStopDuration > 0.f)
	{
		FGameplayEventData StopEvent;
		StopEvent.EventMagnitude = AttackerHitStopDuration;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor( // 이벤트 전달
			Avatar, GameplayTags::Event_Combat_HitStop, StopEvent);
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	
}

void UKDGameplayAbility_AreaBlast::GatherTargets(TArray<FHitResult>& OutHits) const
{
	// 기능 : 타겟수집, 박스 후보 -> 가로 거리 -> 반각 -> 시야 순 필터
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!IsValid(Avatar) || !World) return;
	const FVector Origin = Avatar->GetActorLocation();
	
	// 세로 중심 = 발밑 오프셋 + 높이 절반
	const float HalfHeight = AreaFilter.Height * 0.5f;
	const FVector QueryCenter = Origin + FVector(0.f, 0.f, AreaFilter.HeightOffset + HalfHeight);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	
	// 박스로 넓게 줍고 아래에서 가로 거리로 원기둥 생성
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		QueryCenter,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeBox(FVector(AreaFilter.Radius, AreaFilter.Radius, HalfHeight)),
		Params);
	
	// 부채꼴만 반각 적용 — 원기둥은 전방위
	const bool bUseAngle = (AreaFilter.ShapeType == EKDTargetShapeType::Arc);
	const float CosThreshold = bUseAngle ? FMath::Cos(FMath::DegreesToRadians(AreaFilter.HalfAngle)) : -1.f;
	const FVector Basis = Avatar->GetActorForwardVector().GetSafeNormal2D();
	const float RadiusSq = FMath::Square(AreaFilter.Radius);
	const float DebugLife = 1.f;   // 디버그 표시 시간
	if (bDrawDebug)
	{
		DrawDebugCylinder(World, Origin + FVector(0.f, 0.f, AreaFilter.HeightOffset),
			Origin + FVector(0.f, 0.f, AreaFilter.HeightOffset + AreaFilter.Height),
			AreaFilter.Radius, 24, FColor::Orange, false, DebugLife);
	}
	
	TSet<AActor*> Seen;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		// null / 중복 액터(여러 콜리전 보유) 제거
		AActor* Candidate = Overlap.GetActor();
		if (!IsValid(Candidate) || Seen.Contains(Candidate)) continue;
		Seen.Add(Candidate);
		
		// ASC 없는 대상 제외
		if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate)) continue;
		
		// 박스 모서리 제거 
		const FVector TargetLoc = Candidate->GetActorLocation();
		if (FVector::DistSquared2D(Origin, TargetLoc) > RadiusSq) continue;
		
		// 부채꼴 반각
		const FVector ToTarget = (TargetLoc - Origin).GetSafeNormal2D();
		if (bUseAngle && FVector::DotProduct(Basis, ToTarget) < CosThreshold)
		{
			if (bDrawDebug) DrawDebugLine(World, Origin, TargetLoc, FColor::Silver, false, DebugLife);
			continue;
		}
		
		// 시야 확인 — 벽 차단 시 탈락
		FHitResult Hit;
		Hit.HitObjectHandle = FActorInstanceHandle(Candidate);
		Hit.ImpactPoint = TargetLoc;
		Hit.Location = TargetLoc;
		Hit.ImpactNormal = -ToTarget;
		if (bDrawDebug)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 12.f, 8, FColor::Green, false, DebugLife);
		}
		OutHits.Add(Hit);
	}
}

bool UKDGameplayAbility_AreaBlast::ApplyHit(const FHitResult& Hit)
{
	// 기능 : 대상 1명에 데미지 + 히트 이벤트 + 타격감 큐
	AActor* HitActor = Hit.GetActor();
	UAbilitySystemComponent* AttackerASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!IsValid(HitActor) || !AttackerASC || !TargetASC) return false;
	
	// 아군 타격 통과
	if (UKDAbilityStatics::IsFriendlyFire(AttackerASC, TargetASC))
	{
		return false;
	}
	
	// 데미지 Spec
	const float AttackPower = AttackerASC->GetNumericAttribute(UKDCombatAttributeSet::GetAttackPowerAttribute()) * DamageMultiplier;
	const FGameplayEffectContextHandle Context = UKDAbilityStatics::ApplyDamageEffect(
		AttackerASC, TargetASC, DamageEffectClass, AttackPower, PoiseMultiplier, Hit, GetAvatarActorFromActorInfo());
	
	// 히트 알림
	UKDAbilityStatics::SendHitEvent(HitActor, GetAvatarActorFromActorInfo(), GetAssetTags(), Context, KnockbackMultiplier);
	// i-frame 닷지/사망한 대상엔 타격감 큐 생략
	if (TargetASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Invulnerable)
		|| TargetASC->HasMatchingGameplayTag(GameplayTags::State_Dead))
	{
		return true;
	}
	
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;
	CueParams.RawMagnitude = HitConfirmMagnitude;
	CueParams.EffectContext = Context;
	CueParams.SourceObject = HitConfirmProfile;
	AttackerASC->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_PlayerHitConfirm, CueParams);
	return true;
}
