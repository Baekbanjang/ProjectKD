// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_MeleeTraceBase.h"
#include "GA_PlayerMeleeAttackBase.generated.h"

class UHitConfirmProfile;
// 플레이어 근접 공격 공통 베이스 — 히트스탑 + 타격감 큐 + 락온 자동조준
// 아래 두 계열이 상속, 차이는 몽타주를 어디서 받느냐 하나뿐
//   UGA_PlayerAttackBase    = 콤보 — DA_ComboTree 노드가 AttackMontage를 채움
//   UGA_PlayerOneShotAttack = 단발 — 에디터에서 AttackMontage 직접 지정
UCLASS(Abstract)
class PROJECT_KD_API UGA_PlayerMeleeAttackBase : public UGA_MeleeTraceBase
{
	GENERATED_BODY()

protected:
	// 플레이어 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.08f;

	// 적 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float VictimHitStopDuration = 0.12f;

	// 타격감 큐 크기 — 셰이크/이펙트 배율
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;
	
	// 무기별 타격감 프로필 — CueParams.SourceObject로 전달, BP GC가 무기별 분기
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UHitConfirmProfile> HitConfirmProfile;
	
	// 히트 스탑
	void ApplyHitStop(AActor* Target, float Duration) const;

	// 타격 시 플레이어 전용 HitConfirm 큐 실행
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) override;

	// 락온 중 타겟 방향 자동 조준
	virtual void OnActivated() override;
};
