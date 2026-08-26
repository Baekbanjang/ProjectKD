// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerMelee.h"
#include "KDGameplayAbility_PlayerAirCombo.generated.h"

// 공중 콤보 베이스 — 노드 소비 + 체공 억제
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_PlayerAirCombo : public UKDGameplayAbility_PlayerMelee
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 체공 억제 ON (락온 조준 Super 호출 후).
	virtual void OnActivated() override;

	// 중력 복원 + 막타면 쿨다운 GE. Super 반드시 호출(WeaponTrace task 정리).
	virtual void OnCleanup(bool bWasCancelled) override;

	// 자식 생성자가 지정하는 콤보 입력 태그 — ComboComponent::ProcessInput에 넘김
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	FGameplayTag ComboInputTag;

	// 노드가 GE를 안 주면 이 값 — InstancedPerActor라 직전 값이 남음, 매 시작 복원
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	TSubclassOf<UGameplayEffect> DefaultAirDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DefaultAirDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DefaultAirKnockbackMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DefaultAirPoiseMultiplier = 1.f;


	// 막타 종료 시 부여할 쿨다운 GE. State.Combat.AirComboLock GrantsTag, Duration ~0.6~1.0s.
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	TSubclassOf<UGameplayEffect> AirComboLockGE;

private:
	// 이번 활성화가 막타(콤보 끝)인지 — OnCleanup에서 쿨다운 부여 판정.
	bool bIsFinisher = false;

	// 체공 억제 전 GravityScale 캐시 (InstancedPerActor 대비 매 OnActivated 갱신).
	float OrigGravityScale = 1.f;
	
};
