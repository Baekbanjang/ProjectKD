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

bool UKDAbilityStatics::IsFriendlyFire(const UAbilitySystemComponent* AttackerASC, const UAbilitySystemComponent* TargetASC)
{
	// 기능 : 아군 사격 판정 — 공격자·피격자 둘 다 Team.Enemy
	return AttackerASC && TargetASC
		&& AttackerASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& TargetASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy);
}

FGameplayEffectContextHandle UKDAbilityStatics::ApplyDamageEffect(UAbilitySystemComponent* AttackerASC,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> DamageEffectClass,
	float FinalAttackPower, const FHitResult& Hit, AActor* SourceActor)
{
	// 기능 : 데미지 GE 적용 — Context 생성 + SetByCaller(AttackPower) + ApplyToTarget
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(SourceActor);
	Context.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = AttackerASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (SpecHandle.IsValid())
	{
		// 양수 = IncomingDamage 게이트로 들어갈 데미지
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			SpecHandle, GameplayTags::SetByCaller_AttackPower, FinalAttackPower);
		AttackerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
	}
	return Context;
}

void UKDAbilityStatics::SendHitEvent(AActor* HitActor, AActor* EventInstigator,
	const FGameplayTagContainer& InstigatorTags, const FGameplayEffectContextHandle& Context,
	float KnockbackMultiplier)
{
	// 기능 : Event.Combat.Hit 발신 — 반응은 맞은 쪽이 선택
	// ContextHandle = 방향 넉백·피격 리액션용 충돌 정보
	FGameplayEventData HitEvent;
	HitEvent.Instigator = EventInstigator;
	HitEvent.Target = HitActor;
	HitEvent.InstigatorTags = InstigatorTags;
	HitEvent.ContextHandle = Context;
	HitEvent.EventMagnitude = KnockbackMultiplier;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor, GameplayTags::Event_Combat_Hit, HitEvent);
}
