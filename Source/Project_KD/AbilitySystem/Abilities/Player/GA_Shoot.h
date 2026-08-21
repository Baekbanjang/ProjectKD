// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_ActionBase.h"
#include "GA_Shoot.generated.h"

class UAnimMontage;      
class UGameplayEffect;   
class AKDProjectile;
// 사격 GA — 조준 중 좌클릭
// 상체 슬롯 몽타주 재생 + 발사 노티 프레임에 총알 스폰
UCLASS()
class PROJECT_KD_API UGA_Shoot : public UGA_ActionBase
{
	GENERATED_BODY()

public:
	UGA_Shoot();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> ShootMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	// 탄약 리젠 GE
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot")
	TSubclassOf<UGameplayEffect> RegenBlockEffectClass;
	
	// 스폰할 총알 BP
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot")
	TSubclassOf<AKDProjectile> ProjectileClass;
	
	// 총알이 운반할 데미지 GE
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 총구 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot")
	FName MuzzleSocket = TEXT("Muzzle");

	// 무기 태그
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot")
	FName WeaponTag = TEXT("Gun");

	// 조준 트레이스 사거리
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shoot", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
	float AimTraceRange = 10000.f;

	// 조준점 디버그 그리기 유무
	UPROPERTY(EditDefaultsOnly, Category = "Action|Debug")
	bool bDrawAimDebug = false;

private:
	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnShootEvent(FGameplayEventData Payload);
};
