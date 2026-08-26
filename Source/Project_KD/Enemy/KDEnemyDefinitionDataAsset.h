// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KDEnemyDefinitionDataAsset.generated.h"

class UGameplayAbility;
class UAnimMontage;

// 적 1종의 공격 후보 1개. UBTTask_SelectAttack이 거리밴드 + 가중치로 선택.
// 쿨다운은 GA 자체 Cooldown GE가 관리(여기 중복 X).
USTRUCT(BlueprintType)
struct FEnemyAttackEntry
{
	GENERATED_BODY()

	// 발동할 어빌리티 식별 태그(ASC TryActivateAbilitiesByTag).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Ability"))
	FGameplayTag AbilityTag;

	// 유효 거리 밴드(cm). 플레이어가 [Min, Max]에 있을 때만 후보.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MaxRange = 200.f;

	// 후보 여럿일 때 가중 랜덤 가중치. 클수록 자주.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

// 적 1종의 전체 정의(스탯 + AI거리 + 전투 + 어빌리티 + 공격셋). BP child는 외형만, 수치·행동은 이 에셋 소유.
// AKDEnemyBaseCharacter가 참조, PossessedBy에서 적용. 보스는 페이즈별 에셋 swap 이음새.
UCLASS(BlueprintType)
class PROJECT_KD_API UKDEnemyDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 스탯 (PossessedBy에서 AS base에 적용) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxPoise = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float AttackPower = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float Defense = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxShield = 0.f;

	// --- AI 거리 (캐릭터 게터 경유로 BTService_FindPlayer / kiting 노드가 읽음) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float SightRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	// 활/원거리형 유지 거리. 0=후퇴 안 함(melee). kiting 노드가 GetStandoffRange로 읽음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float StandoffRange = 0.f;

	// 시야 반각(도) — Perception 시야 콘 = 정면 기준 좌우 각각 이 값. 90=전방 180도.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionAngle = 70.f;

	// 시야 상실 후 타겟을 기억하는 시간(초). 경과 시 추격 포기 → 패트롤 복귀. 0=즉시 잊음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float SightMemoryDuration = 4.f;

	// 패트롤 반경(cm) — 스폰(홈) 위치 기준 랜덤 지점 순찰. 0=패트롤 안 함(제자리 Idle).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float PatrolRadius = 800.f;

	// --- 전투 ---
	// 피격 시 공격자 반대로 수평으로 밀리는 거리(cm). 0이면 넉백 없음.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float KnockbackDistance = 60.f;
	
	// 적이 피격당 잃는 Poise(균형). 공격 식별 태그 → 데미지량. 플레이어 공격만 등록(적끼리 friendly fire 무시).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TMap<FGameplayTag, float> PoiseDamageByAttack;

	// 사망 연출 몽타주(선택). 재생 후 랙돌로 안착(하이브리드). 미지정이면 즉시 랙돌.
	// 처형 사망은 처형 피니셔가 죽음 연출 — 이 몽타주 건너뜀.
	// 랙돌 꺾임 발생 시 PhysicsAsset/콜리전 문제 — 에디터에서 수정(코드 아님).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	// 경직(포이즈 브레이크) 몽타주 — 진입 시 재생, 회복 시 정지. 미지정이면 정지 포즈만(기존 동작).
	// StaggerDuration보다 짧으면 끝나고 locomotion 복귀 — 길이 맞추거나 루프 섹션 권장.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> StaggerMontage;

	// 피격 움찔 몽타주 — GA_EnemyHitReact가 피격 방향 섹션(HitF/HitB/HitL/HitR)으로 점프 재생.
	// 섹션 없으면 처음부터 재생. 미지정 = 움찔 없음(HitReact GA를 StartupAbilities에 안 넣는 적과 짝).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	// --- 밸런싱 (캐릭터 게터 경유 read-through) ---
	// 공격 몽타주 재생속도 배수. GA_EnemyWeaponTraceBase::GetEffectiveMontagePlayRate가 곱함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float AttackSpeedMultiplier = 1.f;

	// 전조(윈드업) 엇박 슬로우 배수. ANS_WindupSlow가 SlowRate에 곱함(최종 clamp 0.05~1.0). 작을수록 더 느린 엇박.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "0.05", ClampMax = "2.0"))
	float TelegraphSlowMultiplier = 1.f;

	// 교전 진입 거리(cm) — BTService_RequestAttackToken이 이 안일 때 토큰 요청. 기존 constexpr 700 승격.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "0.0"))
	float EngagementRange = 700.f;

	// 회전 속도(도/초) — 플레이어 쪽으로 도는 yaw 속도. 작을수록 느린 턴. PossessedBy에서 CMC RotationRate에 적용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance", meta = (ClampMin = "30.0", ClampMax = "1080.0"))
	float TurnRate = 360.f;

	// --- 어빌리티 ---
	// 스폰 시 부여(예: UGA_EnemyBasicAttack BP child, ranged GA BP child).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	// 공격 후보셋(거리밴드 + 가중치). UBTTask_SelectAttack이 소비. 보스 = 페이즈별 에셋 swap 이음새.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacks")
	TArray<FEnemyAttackEntry> Attacks;
};
