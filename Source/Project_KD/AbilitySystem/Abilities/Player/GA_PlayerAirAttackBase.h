// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Player/GA_PlayerOneShotAttack.h"
#include "GA_PlayerAirAttackBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_KD_API UGA_PlayerAirAttackBase : public UGA_PlayerOneShotAttack
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

	//  콤보 카운팅 입력 태그.
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	FGameplayTag ComboInputTag;

	// 공중 디폴트 N타. 미매칭 시 콤보 길이로 인덱싱(cap). 인덱스 = 1타째→[0].
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	TArray<TObjectPtr<UAnimMontage>> DefaultAirMontages;

	// 디폴트 N타용 데미지 GE. 매칭 분기는 분기 GE 우선.
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	TSubclassOf<UGameplayEffect> DefaultAirDamageEffectClass;

	// 막타 종료 시 부여할 쿨다운 GE. State.Combat.AirComboLock GrantsTag, Duration ~0.6~1.0s.
	UPROPERTY(EditDefaultsOnly, Category = "Action|AirCombo")
	TSubclassOf<UGameplayEffect> AirComboLockGE;

private:
	// 이번 활성화가 막타(콤보 끝)인지 — OnCleanup에서 쿨다운 부여 판정.
	bool bIsFinisher = false;

	// 체공 억제 전 GravityScale 캐시 (InstancedPerActor 대비 매 OnActivated 갱신).
	float OrigGravityScale = 1.f;
	
};
