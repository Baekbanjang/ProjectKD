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
	// 공격자(플레이어) 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.08f;
	
	// 타격감 큐 크기 — 셰이크/이펙트 배율
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;
	
	// 무기별 타격감 프로필 — CueParams.SourceObject로 전달, BP GC가 무기별 분기
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UHitConfirmProfile> HitConfirmProfile;

	// 일반 자동 조준 사거리
	UPROPERTY(EditDefaultsOnly, Category = "Action|AutoAim", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float AutoAimRange = 500.f;
	
	// 자동 조준 부채꼴 각도 — 180 = 카메라 정면 좌우 90도
	UPROPERTY(EditDefaultsOnly, Category = "Action|AutoAim", meta = (ClampMin = "30.0", ClampMax = "360.0"))
	float AutoAimConeAngle = 180.f;

	// 타격 시 플레이어 전용 HitConfirm 큐 실행
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) override;

	// 공격 시작 시 대상 방향 자동 조준 — 락온 중이면 고정 조준
	virtual void OnActivated() override;
};
