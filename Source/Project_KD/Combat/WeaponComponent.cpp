#include "Combat/WeaponComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
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

	Super::EndPlay(EndPlayReason);
}

void UWeaponComponent::AttachWeaponToSocket(FName SocketName)
{
	if (!WeaponMesh) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerMesh) return;

	WeaponMesh->AttachToComponent(OwnerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

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

	if (bDraw && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Attacking))
	{
		AttachWeaponToHand();
		return;
	}

	FGameplayEventData Data;
	Data.EventTag = GameplayTags::Event_Combat_WeaponToggle;
	Data.Instigator = GetOwner();
	Data.EventMagnitude = bDraw ? 1.0f : 0.0f;
	Data.OptionalObject = bDraw ? CurrentWeapon->DrawMontage : CurrentWeapon->SheathMontage; 
	ASC->HandleGameplayEvent(Data.EventTag, &Data);
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerMesh) return;

	WeaponMesh = NewObject<USkeletalMeshComponent>(Owner, TEXT("WeaponMesh"));
	if (!WeaponMesh) return;

	USkeletalMesh* MeshToUse = CurrentWeapon ? CurrentWeapon->WeaponMesh.Get() : WeaponMeshAsset.Get();
	if (MeshToUse)
	{
		WeaponMesh->SetSkeletalMesh(MeshToUse);
	}


	WeaponMesh->ComponentTags.Add(WeaponComponentTag);
	WeaponMesh->RegisterComponent();
	
	if (bUseSheathing)
	{
		AttachWeaponToSheath();  
		RegisterCombatTagListener();
	}
	else
	{
		AttachWeaponToHand();
	}
}
