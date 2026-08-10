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

FVector UKDAbilityStatics::GetMuzzleLocation(const AActor* Avatar, FName MuzzleSocket)
{
	if (!IsValid(Avatar)) return FVector::ZeroVector;

	if (const ACharacter* Char = Cast<ACharacter>(Avatar))
	{
		if (const USkeletalMeshComponent* Mesh = Char->GetMesh())
		{
			if (MuzzleSocket != NAME_None && Mesh->DoesSocketExist(MuzzleSocket))
				return Mesh->GetSocketLocation(MuzzleSocket);
		}
	}
	
	return Avatar->GetActorLocation();
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
