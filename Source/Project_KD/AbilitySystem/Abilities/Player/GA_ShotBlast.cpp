// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_ShotBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/Attributes/AS_Combat.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"
#include "Combat/Data/HitConfirmProfile.h"
#include "Combat/LockOnComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "Player/KDPlayerCharacter.h"

UGA_ShotBlast::UGA_ShotBlast()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bRetriggerInstancedAbility = true;   // 한 몽타주 연속 사격
	
	// 노티 이벤트로 자동 활성
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = GameplayTags::Event_Montage_ShotBlast;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGA_ShotBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 기능 : 총구 콘 히트스캔 1회 후 즉시 종료
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

	// 총구 위치
	const FVector MuzzleLoc = UKDAbilityStatics::GetMuzzleLocation(Avatar, MuzzleSocket);
	
	// 발사 방향 = 락온 타겟 우선 | 폴백 = 액터 전방
	FVector ShotDir = Avatar->GetActorForwardVector();
	if (AKDPlayerCharacter* PC = Cast<AKDPlayerCharacter>(Avatar))
	{
		if (ULockOnComponent* LockOn = PC->GetLockOnComponent())
		{
			if (const AActor* Target = LockOn->GetLockedTarget())
			{
				const FVector ToTarget = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
				if (!ToTarget.IsNearlyZero()) ShotDir = ToTarget;
			}
		}
	}

	TArray<FHitResult> Hits;
	GatherTargets(MuzzleLoc, ShotDir, Hits);
	bool bAnyHit = false;
	for (const FHitResult& Hit : Hits)
	{
		if (ApplyHit(Hit)) bAnyHit = true;
	}
	
	// 히트스톱 1회 — 다중 명중에도 중복 X
	if (bAnyHit && AttackerHitStopDuration > 0.f)
	{
		FGameplayEventData StopEvent;
		StopEvent.EventMagnitude = AttackerHitStopDuration;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Avatar, GameplayTags::Event_Combat_HitStop, StopEvent);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_ShotBlast::GatherTargets(const FVector& MuzzleLoc, const FVector& ShotDir, TArray<FHitResult>& OutHits) const
{
	// 기능 : 사거리 구체 후보 -> 콘 각도 -> 시야 순 필터
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!IsValid(Avatar) || !World) return;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	
	// 후보 수집 — 총구 중심 사거리 구체
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, MuzzleLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ShotRange), Params);
	
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ShotHalfAngle));
	const float DebugLife = 1.f;   // 디버그 표시 시간
	const float ConeRad = FMath::DegreesToRadians(ShotHalfAngle);
	if (bDrawDebug)
	{
		DrawDebugCone(World, MuzzleLoc, ShotDir, ShotRange, ConeRad, ConeRad,
			16, FColor::Yellow, false, DebugLife);
	}
	
	TSet<AActor*> Seen;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!IsValid(Candidate) || Seen.Contains(Candidate)) continue;
		Seen.Add(Candidate);
		
		// ASC 없는 대상 제외
		if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate)) continue;
		const FVector TargetLoc = Candidate->GetActorLocation();
		
		// 콘 각도
		const FVector ToTarget = (TargetLoc - MuzzleLoc).GetSafeNormal();
		if (FVector::DotProduct(ShotDir, ToTarget) < CosHalfAngle)
		{
			if (bDrawDebug) DrawDebugLine(World, MuzzleLoc, TargetLoc, FColor::Silver, false, DebugLife);
			continue;
		}
		
		// 시야 확인 — 벽 차단 시 탈락
		FHitResult Hit;
		const bool bTraced = World->LineTraceSingleByChannel(Hit, MuzzleLoc, TargetLoc, ECC_Visibility, Params);
		if (bTraced && Hit.GetActor() != Candidate)
		{
			if (bDrawDebug) DrawDebugLine(World, MuzzleLoc, Hit.ImpactPoint, FColor::Red, false, DebugLife);
			continue;
		}
		
		// 트레이스 미검출 시 액터 중심으로 대체
		if (!bTraced)
		{
			Hit.HitObjectHandle = FActorInstanceHandle(Candidate);
			Hit.ImpactPoint = TargetLoc;
			Hit.Location = TargetLoc;
			Hit.ImpactNormal = -ShotDir;
		}
		
		if (bDrawDebug)
		{
			DrawDebugLine(World, MuzzleLoc, Hit.ImpactPoint, FColor::Green, false, DebugLife);
			DrawDebugSphere(World, Hit.ImpactPoint, 12.f, 8, FColor::Green, false, DebugLife);
		}
		OutHits.Add(Hit);
	}
}

bool UGA_ShotBlast::ApplyHit(const FHitResult& Hit)
{
	// 기능 : 대상 1명에 데미지 + 히트 이벤트 + 타격감 큐
	AActor* HitActor = Hit.GetActor();
	UAbilitySystemComponent* AttackerASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!IsValid(HitActor) || !AttackerASC || !TargetASC) return false;
	
	// 아군 사격 통과
	if (AttackerASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& TargetASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy))
	{
		return false;
	}
	
	// 데미지 Spec
	const float AttackPower = AttackerASC->GetNumericAttribute(UAS_Combat::GetAttackPowerAttribute());
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());
	Context.AddHitResult(Hit);
	FGameplayEffectSpecHandle SpecHandle = AttackerASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (SpecHandle.IsValid())
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			SpecHandle, GameplayTags::SetByCaller_AttackPower, AttackPower);
		AttackerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
	}
	
	// 히트 알림 — 반응은 맞은 쪽이 선택
	FGameplayEventData HitEvent;
	HitEvent.Instigator = GetAvatarActorFromActorInfo();
	HitEvent.Target = HitActor;
	HitEvent.InstigatorTags = GetAssetTags();
	HitEvent.ContextHandle = Context;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor, GameplayTags::Event_Combat_Hit, HitEvent);
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
