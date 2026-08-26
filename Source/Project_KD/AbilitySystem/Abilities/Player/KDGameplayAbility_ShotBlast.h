// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/KDGameplayAbility.h"
#include "KDGameplayAbility_ShotBlast.generated.h"

class UKDHitConfirmProfile;
// 콤보 속 총 발사 판정 GA 
// AN_ShotBlast의 Event.Montage.ShotBlast로 자동 활성 후 즉시 종료
// 판정 = 총구 콘 히트스캔. Overlap 수집 -> 각도 필터 -> 시야 확인
UCLASS()
class PROJECT_KD_API UKDGameplayAbility_ShotBlast : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_ShotBlast();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 총구 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot")
	FName MuzzleSocket = TEXT("Muzzle");

	// 무기 태그
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot")
	FName WeaponTag = TEXT("Gun");
	
	// 사거리
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot", meta = (ClampMin = "100.0", ClampMax = "3000.0"))
	float ShotRange = 500.f;
	
	// 총알 확산 반(Half)각 — 총구 정면 기준
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot", meta = (ClampMin = "1.0", ClampMax = "89.0"))
	float ShotHalfAngle = 20.f;

	// 데미지 배수
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DefaultShotDamageMultiplier = 0.8f;
	
	float ShotDamageMultiplier = 0.8f;

	// 넉백 배수
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DefaultShotKnockbackMultiplier = 1.5f;
	
	float ShotKnockbackMultiplier = 1.5f;

	// 자동 조준 범위 각도 — 180 = 정면 좌우 90도
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot", meta = (ClampMin = "30.0", ClampMax = "360.0"))
	float AutoAimConeAngle = 180.f;
	
	// 몸 정면 기준 조준 한계각
	UPROPERTY(EditDefaultsOnly, Category = "Action|Shot", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float BodyAimLimitAngle = 60.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	// 공격자 히트 스탑 시간
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float AttackerHitStopDuration = 0.06f;
	
	// 타격감 큐 크기
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitConfirmMagnitude = 1.0f;
	
	// 무기별 타격감 DA
	UPROPERTY(EditDefaultsOnly, Category = "Action|HitStop")
	TObjectPtr<UKDHitConfirmProfile> HitConfirmProfile;
	
	UPROPERTY(EditDefaultsOnly, Category = "Action|Debug")
	bool bDrawDebug = false;

private:
	// 범위 안 유효 타겟 수집
	void GatherTargets(const FVector& Origin, const FVector& ShotDir, float HalfAngle, TArray<FHitResult>& OutHits) const;
	
	// 대상 1명 처리 — 데미지 + 히트 이벤트 + 타격감
	bool ApplyHit(const FHitResult& Hit);
};
