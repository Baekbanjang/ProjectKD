// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/KDProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "KDGameplayTags.h"

AKDProjectile::AKDProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(12.f);
	// 프로파일 Projectile — Pawn | Destructible 만 Overlap, 벽은 Block
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;   // 직선 비행

	InitialLifeSpan = 5.f;   // 안전망
}

void AKDProjectile::InitProjectile(const FGameplayEffectSpecHandle& InDamageSpec,
	UAbilitySystemComponent* InInstigatorASC, const FGameplayTagContainer& InInstigatorTags)
{
	DamageSpec = InDamageSpec;
	InstigatorASC = InInstigatorASC;
	InstigatorTags = InInstigatorTags;
}

void AKDProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	Destroy();
}

void AKDProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 발사자 자기 자신과는 충돌 X
	if (AActor* Shooter = GetInstigator())
		CollisionSphere->IgnoreActorWhenMoving(Shooter, true);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AKDProjectile::OnSphereOverlap);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AKDProjectile::OnProjectileStop);
}

void AKDProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!IsValid(OtherActor) || OtherActor == GetInstigator()) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	// ASC 없는 대상 — 소멸
	if (!TargetASC)
	{
		Destroy();
		return;
	}

	// 아군 사격 통과
	if (InstigatorASC.IsValid()
		&& InstigatorASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& TargetASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy))
	{
		return;
	}

	// 데미지 적용
	if (DamageSpec.IsValid())
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data);

	// 히트 알림 — 반응은 맞은 쪽이 선택
	FGameplayEventData HitEvent;
	HitEvent.Instigator = GetInstigator();
	HitEvent.Target = OtherActor;
	HitEvent.InstigatorTags = InstigatorTags;
	HitEvent.ContextHandle = DamageSpec.IsValid() ? DamageSpec.Data->GetContext() : FGameplayEffectContextHandle();
	HitEvent.EventMagnitude = KnockbackMultiplier;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, GameplayTags::Event_Combat_Hit, HitEvent);

	Destroy();
}
