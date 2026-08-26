// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_EnemyRangedAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;
class AKDProjectile;

// 원거리 잡몹(단검도적 투척 / 활도적) 공격. WeaponTrace 아님 — 발사체를 던진다.
// 몽타주 재생 → 던지는 프레임(Event.Montage.TraceBegin 재사용)에 발사체 1발 스폰.
// 발사체가 데미지 GE를 Context로 운반 → IncomingDamage 게이트 재사용. 리딩/탄도 없음.
UCLASS(Abstract)
class PROJECT_KD_API UKDGameplayAbility_EnemyRangedAttack : public UKDGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	// 스폰할 발사체. 비주얼/속도 = BP child.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Ranged")
	TSubclassOf<AKDProjectile> ProjectileClass;

	// 발사체가 운반할 데미지 GE(근접과 동일 GE 재사용 가능). AttackPower SetByCaller로 채움.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Ranged")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 발사체 발사 소켓(손/활). 없으면 actor 위치.
	UPROPERTY(EditDefaultsOnly, Category = "Action|Ranged")
	FName MuzzleSocket = TEXT("Muzzle");

private:
	UFUNCTION() void OnReleaseProjectile(FGameplayEventData Payload);

	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();
};
