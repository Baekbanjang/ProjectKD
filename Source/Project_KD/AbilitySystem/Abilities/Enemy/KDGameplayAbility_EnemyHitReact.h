#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_EnemyHitReact.generated.h"

// 피격 움찔 GA — Event.Combat.HitReact(AS_Combat이 데미지 적용 시 발신)로 자동 트리거.
// 공격자(EffectCauser) 위치 → 피격 각도 → DA HitReactMontage의 섹션(HitF/HitB/HitL/HitR) 분기 재생.
// 발동 차단: SuperArmor(공격 GA Owned Tag — 패턴은 평타로 안 끊김) / 경직(넉다운이 몸 소유) / 사망.
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_EnemyHitReact : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_EnemyHitReact();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	// 몽타주 완료/인터럽트/취소 공통 → EndAbility.
	UFUNCTION()
	void OnMontageFinished();
};
