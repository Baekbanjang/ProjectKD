// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "KDAbilityStatics.generated.h"

class AKDProjectile;
class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

// GAS 공용 static 헬퍼
UCLASS()
class PROJECT_KD_API UKDAbilityStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 총구 트랜스폼 — 무기 메시 소켓 | 캐릭터 메시 소켓 | 액터 순
	// WeaponTag 비우면 무기 메시  생략
	static FTransform GetMuzzleTransform(const AActor* Avatar, FName MuzzleSocket, FName WeaponTag = NAME_None);

	// 공격력 실은 데미지 Spec 운반 발사체 스폰
	static AKDProjectile* SpawnDamageProjectile(
		UAbilitySystemComponent* InstigatorASC,
		AActor* Avatar,
		TSubclassOf<AKDProjectile> ProjectileClass,
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		const FVector& SpawnLoc,
		const FRotator& SpawnRot,
		const FGameplayTagContainer& AbilityTags);

	// 아군 사격 유무 — 공격자·피격자 둘 다 Team.Enemy면 true
	static bool IsFriendlyFire(const UAbilitySystemComponent* AttackerASC, const UAbilitySystemComponent* TargetASC);

	// 데미지 GE 적용 — Context 생성 + SetByCaller(AttackPower) + Poise 배수+ ApplyToTarget. 반환 = 생성한 Context
	static FGameplayEffectContextHandle ApplyDamageEffect(UAbilitySystemComponent* AttackerASC,
		UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> DamageEffectClass,
		float FinalAttackPower, float PoiseMultiplier, const FHitResult& Hit, AActor* SourceActor,
		const UGameplayAbility* SourceAbility = nullptr);

	// Event.Combat.Hit 발신 — 5필드 규약 (EventMagnitude = 넉백 배수)
	static void SendHitEvent(AActor* HitActor, AActor* EventInstigator,
		const FGameplayTagContainer& InstigatorTags, const FGameplayEffectContextHandle& Context,
		float KnockbackMultiplier);
};
