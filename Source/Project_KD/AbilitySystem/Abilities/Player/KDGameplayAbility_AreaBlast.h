// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "Combat/Data/KDTargetFilter.h"
#include "KDGameplayAbility_AreaBlast.generated.h"

class UKDHitConfirmProfile;
/**
 * 
 */
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_AreaBlast : public UKDGameplayAbility
{
	GENERATED_BODY()
public:
	UKDGameplayAbility_AreaBlast();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	// 대상 수집 조건
	UPROPERTY(EditDefaultsOnly, Category = "Action|Area")
	FKDTargetFilter AreaFilter;
	
	// 데미지 배수
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DamageMultiplier = 1.5f;
	
	// Poise 배수
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float PoiseMultiplier = 1.f;
	
	// 넉백 배수
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float KnockbackMultiplier = 2.f;

	// Poise 포함이 된 DamageEffectClass
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	// 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.1f;
	
	// 타격감 큐 크기
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.5f;
	
	// 무기별 타격감 DA
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UKDHitConfirmProfile> HitConfirmProfile;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action|Debug")
	bool bDrawDebug = false;
private:
	// 범위 안 유효 타겟 수집
	void GatherTargets(TArray<FHitResult>& OutHits) const;
	
	bool ApplyHit(const FHitResult& Hit);
};
