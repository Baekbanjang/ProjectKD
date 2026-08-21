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

// 발사체 액터 — 적 원거리 | 플레이어 사격 공용
// 데미지 GE Spec 을 들고 날아가서 맞은 액터 ASC 에 적용 + Event.Combat.Hit 전송
UCLASS(Abstract)
class PROJECT_KD_API AKDProjectile : public AActor
{
	GENERATED_BODY()

public:
	AKDProjectile();

	// 데미지 Spec + 발사자 정보 전달
	void InitProjectile(const FGameplayEffectSpecHandle& InDamageSpec,
		UAbilitySystemComponent* InInstigatorASC,
		const FGameplayTagContainer& InInstigatorTags);

	// 퍼펙트 회피 대상 여부
	bool IsPerfectDodgeable() const { return bPerfectDodgeable; }

protected:
	// 충돌 콜백 연결
	virtual void BeginPlay() override;

	// Pawn 겹침 처리
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	// 벽 충돌 처리
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	// 충돌 범위 겸 루트
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	// 직선 비행
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 퍼펙트 회피 대상 여부
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bPerfectDodgeable = true;

	// 넉백 배수 
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float KnockbackMultiplier = 1.f;

private:
	// 적용할 데미지
	FGameplayEffectSpecHandle DamageSpec;
	
	// 발사자 ASC
	TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC;
	
	// 발사 GA 태그
	FGameplayTagContainer InstigatorTags;
};
