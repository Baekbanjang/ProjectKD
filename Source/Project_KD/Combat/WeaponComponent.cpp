#include "Combat/WeaponComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponDataAsset.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::AttachWeaponToHand()
{
	const FName HandSocket = CurrentWeapon ? CurrentWeapon->HandSocketName : AttachSocketName;
	AttachWeaponToSocket(HandSocket);
}

void UWeaponComponent::AttachWeaponToSheath()
{
	const FName SheathSocket = CurrentWeapon ? CurrentWeapon->SheathSocketName : FName(TEXT("weapon_back"));
	AttachWeaponToSocket(SheathSocket);
}

void UWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InCombatTagHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_InCombat, EGameplayTagEventType::NewOrRemoved).Remove(InCombatTagHandle);
		}
		InCombatTagHandle.Reset();
	}

	if (InActionTagHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_InAction, EGameplayTagEventType::NewOrRemoved).Remove(InActionTagHandle);
		}
		InActionTagHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UWeaponComponent::AttachWeaponToSocket(FName SocketName)
{
	if (!WeaponMesh) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerMesh) return;

	WeaponMesh->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

	// GripPoint 자동 정렬 — attach 끝난 다음에 실행해야 트랜스폼이 정확히 반영
	if (WeaponMesh->DoesSocketExist(TEXT("GripPoint")))
	{
		const FTransform GripLocal = WeaponMesh->GetSocketTransform(TEXT("GripPoint"), RTS_Component);
		WeaponMesh->SetRelativeTransform(GripLocal.Inverse());
	}
}

void UWeaponComponent::RegisterCombatTagListener()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		// ASC가 아직 준비 안 됨(PlayerState에 늦게 붙음) -> 다음 틱 재시도
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UWeaponComponent::RegisterCombatTagListener);
		}
		return;
	}
	
	InCombatTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_InCombat, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UWeaponComponent::OnInCombatTagChanged);
}

void UWeaponComponent::OnInCombatTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!CurrentWeapon) return; // 데이터에셋 없으면 재생할 몽타주도 없음

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;

	const bool bDraw = (NewCount > 0);

	// 전투 상태 급히 발생 시 바로 전투
	if (bDraw && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InAction))
	{
		AttachWeaponToHand();
		return;
	}

	FGameplayEventData Data;
	Data.EventTag = GameplayTags::Event_Combat_WeaponToggle;
	Data.Instigator = GetOwner();
	Data.EventMagnitude = bDraw ? 1.0f : 0.0f;
	const FEquipMontageSet& MontageSet = bDraw ? CurrentWeapon->DrawMontages : CurrentWeapon->SheathMontages;
	Data.OptionalObject = SelectEquipMontage(MontageSet);
	
	// 다중 무기: 몽타주 재생 트리거는 1개 컴포넌트만 (총 쪽 false). 부착은 노티가 전체 순회
	if (bBroadcastsToggleEvent)
	{
		ASC->HandleGameplayEvent(Data.EventTag, &Data);
	}
}

UAnimMontage* UWeaponComponent::SelectEquipMontage(const FEquipMontageSet& Set) const
{
	const AActor* Owner = GetOwner();
	const float Speed = Owner ? Owner->GetVelocity().Size2D() : 0.f;
	if (Speed < WalkSpeedThreshold) return Set.Idle;
	if (Speed < RunSpeedThreshold)  return Set.Walk;
	return Set.Run;
}

void UWeaponComponent::RegisterInActionTagListener()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC)
	{
		// ASC가 아직 준비 안 됨(PlayerState에 늦게 붙음) -> 다음 틱 재시도
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(this, &UWeaponComponent::RegisterInActionTagListener);
		}
		return;
	}
	InActionTagHandle = ASC->RegisterGameplayTagEvent(GameplayTags::State_Combat_InAction, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UWeaponComponent::OnInActionTagChanged);
}

void UWeaponComponent::OnInActionTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0) return;
	AttachWeaponToHand();
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerMesh) return;

	// 무기 컴포넌트 2개면 서브오브젝트 이름 충돌 방지 — 태그 기반 유니크 이름
	const FName MeshName(*FString::Printf(TEXT("WeaponMesh_%s"), *WeaponComponentTag.ToString()));

	// 스태틱에서 무기면 StaticMeshComponent, 아니면 기존 스켈레탈 경로
	UStaticMesh* StaticToUse = CurrentWeapon ? CurrentWeapon->WeaponStaticMesh.Get() : nullptr;

	if (StaticToUse)
	{
		UStaticMeshComponent* SMComp = NewObject<UStaticMeshComponent>(Owner, MeshName);
		if (!SMComp) return;
		SMComp->SetStaticMesh(StaticToUse);
		WeaponMesh = SMComp;
	}
	else
	{
		USkeletalMeshComponent* SkelComp = NewObject<USkeletalMeshComponent>(Owner, MeshName);
		if (!SkelComp) return;

		USkeletalMesh* MeshToUse = CurrentWeapon ? CurrentWeapon->WeaponMesh.Get() : WeaponMeshAsset.Get();
		if (MeshToUse)
		{
			SkelComp->SetSkeletalMesh(MeshToUse);
		}
		WeaponMesh = SkelComp;
	}

	WeaponMesh->ComponentTags.Add(WeaponComponentTag);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->RegisterComponent();
	if (bUseSheathing)
	{
		AttachWeaponToSheath();
		RegisterCombatTagListener();
		RegisterInActionTagListener();
	}
	else
	{
		AttachWeaponToHand();
	}
}
