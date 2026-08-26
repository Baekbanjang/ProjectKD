// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerMelee.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"
#include "KDGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_MeleeTrace.h"
#include "AbilitySystem/Combo/KDComboComponent.h"
#include "AbilitySystem/Combo/KDComboTreeDataAsset.h"
#include "Combat/Data/KDHitConfirmProfile.h"

#if !UE_BUILD_SHIPPING
// 개발용 접근 판정 표시 스위치 — 콘솔 KD.ShowApproach 1
static TAutoConsoleVariable<int32> CVarShowApproach(
	TEXT("KD.ShowApproach"), 0,
	TEXT("접근 워프 거리 온스크린 표시 유무"), ECVF_Cheat);
#endif


void UKDGameplayAbility_PlayerMelee::OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit)
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
	const UKDAnimNotifyState_MeleeTrace* Window = GetActiveWindow();
	if (Window && Window->bIgnoreHitStop) { return; }
	
	// 정지 주체는 공격자 HitStopComponent
	FGameplayEventData StopEvent;
	StopEvent.EventMagnitude = AttackerHitStopDuration;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetAvatarActorFromActorInfo(), GameplayTags::Event_Combat_HitStop, StopEvent);
}

const FComboNode* UKDGameplayAbility_PlayerMelee::ApplyComboNode(FGameplayTag InputTag, EComboContext Context,
	TSubclassOf<UGameplayEffect> DefaultGE, float DefaultDamageMul, float DefaultKnockbackMul, float DefaultPoiseMul)
{
	// 기능 : 콤보 노드 소비 — 몽타주·데미지 GE·계수 결정, 노드 없으면 기본값
	UKDComboComponent* Combo = GetComboComponentFromActorInfo();

	const FComboNode* Node = IsValid(Combo)
		? Combo->ProcessInput(InputTag, Context)
		: nullptr;

	// 매 시작에 디폴트 복원 — 직전 값이 다음 활성화까지 남는 것 막음
	DamageEffectClass = DefaultGE;
	DamageMultiplier = DefaultDamageMul;
	KnockbackMultiplier = DefaultKnockbackMul;
	PoiseMultiplier = DefaultPoiseMul;

	if (Node)
	{
		// 노드 = 이번 콤보
		AttackMontage = IsValid(Node->Montage) ? Node->Montage : nullptr; // 노드의 몽타주 GA 변수에 대입
		if (!AttackMontage)
		{
			const TCHAR* LogPrefix = (Context == EComboContext::Air) ? TEXT("[KD] Air combo node") : TEXT("[KD] Combo node");
			UE_LOG(LogTemp, Warning, TEXT("%s '%s' 몽타주 미지정"), LogPrefix, *Node->NodeId.ToString());
		}

		if (Node->DamageEffectClass)
		{
			DamageEffectClass = Node->DamageEffectClass;
		}
		if (Node->DamageMultiplier > 0.f)
		{
			DamageMultiplier = Node->DamageMultiplier;
		}
		if (Node->KnockbackMultiplier > 0.f)
		{
			KnockbackMultiplier = Node->KnockbackMultiplier;
		}
		if (Node->PoiseMultiplier > 0.f)
		{
			PoiseMultiplier = Node->PoiseMultiplier;
		}
	}
	else
	{
		// 트리에서 못 찾음 = 데이터 문제, 몽타주 없이 두면 부모가 EndAbility
		AttackMontage = nullptr;
	}

	return Node;
}

void UKDGameplayAbility_PlayerMelee::OnActivated()
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
	if (GEngine && CVarShowApproach.GetValueOnGameThread() > 0)
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
