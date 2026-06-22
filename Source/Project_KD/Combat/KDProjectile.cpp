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
	// 전용 Projectile 프로파일(collision-channels.md) — Pawn/Destructible만 Overlap, 월드 지오메트리는
	// Block(벽 통과 방지), Camera Ignore. OverlapAll은 트리거 볼륨 등 무관 오버랩에도 반응했음.
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;   // 직선 비행 기본(BP가 arc 추가 가능)

	InitialLifeSpan = 5.f;   // 안전망 — 아무것도 안 맞으면 자동 소멸
}

void AKDProjectile::InitProjectile(const FGameplayEffectSpecHandle& InDamageSpec,
	UAbilitySystemComponent* InInstigatorASC, const FGameplayTagContainer& InInstigatorTags)
{
	DamageSpec = InDamageSpec;
	InstigatorASC = InInstigatorASC;
	InstigatorTags = InInstigatorTags;

	// 발사자 자기 자신과는 절대 충돌 안 함.
	if (AActor* Shooter = GetInstigator())
		CollisionSphere->IgnoreActorWhenMoving(Shooter, true);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AKDProjectile::OnSphereOverlap);

	// 벽 등 블로킹 히트로 비행이 멈추면 소멸(박히는/부서지는 비주얼은 BP 몫).
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AKDProjectile::OnProjectileStop);
}

void AKDProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	Destroy();
}

void AKDProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!IsValid(OtherActor) || OtherActor == GetInstigator()) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	// ASC 없음 = 월드 지오메트리: 그냥 소멸(박히는/부서지는 비주얼은 BP 몫).
	if (!TargetASC)
	{
		Destroy();
		return;
	}

	// Faction 게이트(근접 OnWeaponHit과 동일): 적 발사체는 다른 적을 안 맞춤 — 통과시키고 계속 비행.
	if (InstigatorASC.IsValid()
		&& InstigatorASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& TargetASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy))
	{
		return;
	}

	// 운반한 데미지 Spec을 IncomingDamage 관문에 적용(근접과 동일 경로). 발사자 생존과 무관하게 적용.
	if (DamageSpec.IsValid())
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data);

	// 공격 식별 태그를 실은 보편 히트 이벤트 전송 → victim의 Event.Combat.Hit 핸들러가 반응 결정(디커플, 근접과 동일).
	FGameplayEventData HitEvent;
	HitEvent.Instigator = GetInstigator();
	HitEvent.Target = OtherActor;
	HitEvent.InstigatorTags = InstigatorTags;
	HitEvent.ContextHandle = DamageSpec.IsValid() ? DamageSpec.Data->GetContext() : FGameplayEffectContextHandle();
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, GameplayTags::Event_Combat_Hit, HitEvent);

	Destroy();
}
