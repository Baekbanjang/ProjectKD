// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "KDAbilityStatics.generated.h"

class AKDProjectile;
class UAbilitySystemComponent;
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
};
