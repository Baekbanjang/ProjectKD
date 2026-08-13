// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Library/KDAbilityStatics.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AS_Combat.h"
#include "Combat/KDProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "KDGameplayTags.h"
#include "Combat/WeaponComponent.h"

FTransform UKDAbilityStatics::GetMuzzleTransform(const AActor* Avatar, FName MuzzleSocket, FName WeaponTag)
{
	if (!IsValid(Avatar)) return FTransform::Identity;

	// 무기 메시 — 검/총 구분: WeaponComponentTag
	if (WeaponTag != NAME_None)
	{
		TArray<UWeaponComponent*> Weapons;
		Avatar->GetComponents<UWeaponComponent>(Weapons);
		for (const UWeaponComponent* Weapon : Weapons)
		{
			if (!IsValid(Weapon) || Weapon->GetWeaponComponentTag() != WeaponTag) continue;
			const UMeshComponent* Mesh = Weapon->GetWeaponMesh();
			if (Mesh && Mesh->DoesSocketExist(MuzzleSocket))
			{
				return Mesh->GetSocketTransform(MuzzleSocket);
			}
		}
	}

	// 폴백 — 캐릭터 메시 소켓
	if (const ACharacter* Char = Cast<ACharacter>(Avatar))
	{
		if (const USkeletalMeshComponent* Mesh = Char->GetMesh())
		{
			if (Mesh->DoesSocketExist(MuzzleSocket))
			{
				return Mesh->GetSocketTransform(MuzzleSocket);
			}
		}
	}
	return Avatar->GetActorTransform();
}

AKDProjectile* UKDAbilityStatics::SpawnDamageProjectile(
	UAbilitySystemComponent* InstigatorASC,
	AActor* Avatar,
	TSubclassOf<AKDProjectile> ProjectileClass,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	const FVector& SpawnLoc,
	const FRotator& SpawnRot,
	const FGameplayTagContainer& AbilityTags)
{
	if (!InstigatorASC || !IsValid(Avatar) || !ProjectileClass || !DamageEffectClass) return nullptr;

	UWorld* World = Avatar->GetWorld();
	if (!World) return nullptr;

	// 데미지 Spec
	const float AttackPower = InstigatorASC->GetNumericAttribute(UAS_Combat::GetAttackPowerAttribute());
	FGameplayEffectContextHandle Context = InstigatorASC->MakeEffectContext();
	Context.AddSourceObject(Avatar);

	FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (!SpecHandle.IsValid()) return nullptr;

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle, GameplayTags::SetByCaller_AttackPower, AttackPower);

	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Cast<APawn>(Avatar);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AKDProjectile* Projectile = World->SpawnActor<AKDProjectile>(ProjectileClass, SpawnLoc, SpawnRot, Params);
	
	if (Projectile)
		Projectile->InitProjectile(SpecHandle, InstigatorASC, AbilityTags);

	return Projectile;
}
