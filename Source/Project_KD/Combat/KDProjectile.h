// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"      // FGameplayEffectSpecHandle
#include "GameplayTagContainer.h"
#include "KDProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;

// 적 원거리 공격 발사체(단검 투척 / 화살 공용).
// 발사 GA의 데미지 GE Spec을 운반해 충돌 시 타겟 ASC에 적용, Event.Combat.Hit 전송(근접과 동일 계약).
// 비주얼·속도·수명은 BP child. C++ = 충돌 + 이동 + GAS 적용만.
UCLASS(Abstract)
class PROJECT_KD_API AKDProjectile : public AActor
{
	GENERATED_BODY()

public:
	AKDProjectile();

	// 발사 GA가 스폰 직후 1회 호출. Spec/Instigator 운반시키고 actor forward로 비행.
	// InstigatorTags = 발사 GA의 AssetTags(Ability.Enemy.Grunt.Attack.Ranged) → Event.Combat.Hit에 실음.
	void InitProjectile(const FGameplayEffectSpecHandle& InDamageSpec,
		UAbilitySystemComponent* InInstigatorASC,
		const FGameplayTagContainer& InInstigatorTags);

	bool IsPerfectDodgeable() const { return bPerfectDodgeable; }

protected:
	// 충돌 시: faction 게이트 → GE Spec 적용 → Event.Combat.Hit → Destroy. (자기 발사자 무시)
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	// 블로킹 히트(벽)로 정지 시 소멸 — Projectile 프로파일은 월드 지오메트리를 Block.
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 퍼펙트 닷지 가능한 발사체 여부
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bPerfectDodgeable = true;

private:
	FGameplayEffectSpecHandle DamageSpec;
	TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC;
	FGameplayTagContainer InstigatorTags;
};
