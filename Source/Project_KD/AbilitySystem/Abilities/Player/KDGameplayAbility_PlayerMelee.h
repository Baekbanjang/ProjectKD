// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/KDGameplayAbility_MeleeTrace.h"
#include "Combat/Data/KDTargetFilter.h"
#include "KDGameplayAbility_PlayerMelee.generated.h"

class UKDHitConfirmProfile;
struct FComboNode;
enum class EComboContext : uint8;
// 플레이어 근접 공격 공통 베이스 — 히트스탑 + 타격감 큐 + 락온 자동조준
// 자식의 차이는 몽타주를 어디서 받느냐 하나뿐
//   콤보 계열   UKDGameplayAbility_PlayerCombo | UKDGameplayAbility_PlayerAirCombo — 노드가 AttackMontage 를 채움
//   단발 계열   UKDGameplayAbility_SprintAttack | UKDGameplayAbility_CounterThrust — 에디터에서 AttackMontage 직접 지정
UCLASS(Abstract)
class PROJECT_KD_API UKDGameplayAbility_PlayerMelee : public UKDGameplayAbility_MeleeTrace
{
	GENERATED_BODY()

protected:
	// 공격자(플레이어) 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.08f;
	
	// 타격감 큐 크기 — 셰이크/이펙트 배율
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;
	
	// 무기별 타격감 프로필 — CueParams.SourceObject로 전달, BP GC가 무기별 분기
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UKDHitConfirmProfile> HitConfirmProfile;

	// 자동 조준 탐색 조건 
	UPROPERTY(EditDefaultsOnly, Category = "Action|AutoAim")
	FKDTargetFilter AutoAimFilter;
	
	// 자동 조준 최대 회전 각도 
	UPROPERTY(EditDefaultsOnly, Category = "Action|AutoAim", meta = (ClampMin = "45.0", ClampMax = "180.0"))
	float MaxAutoAimTurnAngle = 135.f;
	
	// 몽타주의 Motion Warping 노티 접근점 이름
	UPROPERTY(EditDefaultsOnly, Category = "Action|Approach")
	FName ApproachWarpName;
	
	// 타겟 앞에서 멈출 거리
	UPROPERTY(EditDefaultsOnly, Category = "Action|Approach", meta = (ClampMin = "0.0"))
	float ApproachStopDistance = 200.f;
	
	// 접근 가능 최대 거리 — 초과 시 제자리
	UPROPERTY(EditDefaultsOnly, Category = "Action|Approach", meta = (ClampMin = "0.0"))
	float MaxApproachRange = 700.f;

	// 콤보 노드 소비 — 노드 값으로 몽타주·데미지 GE·계수 결정, 노드 없으면 기본값
	const FComboNode* ApplyComboNode(FGameplayTag InputTag, EComboContext Context,
		TSubclassOf<UGameplayEffect> DefaultGE, float DefaultDamageMul, float DefaultKnockbackMul, float DefaultPoiseMul);

	// 타격 시 플레이어 전용 HitConfirm 큐 실행
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) override;

	// 공격 시작 시 대상 방향 자동 조준 — 락온 중이면 고정 조준
	virtual void OnActivated() override;
};
