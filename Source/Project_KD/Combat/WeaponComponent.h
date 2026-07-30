#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

struct FEquipMontageSet;
class UWeaponDataAsset;
class UMeshComponent;

UCLASS(ClassGroup=(Weapon), meta=(BlueprintSpawnableComponent))
class PROJECT_KD_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	UMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	// 노티가 무기 식별에 사용
	FName GetWeaponComponentTag() const { return WeaponComponentTag; }

	void AttachWeaponToHand();   
	void AttachWeaponToSheath();
	
protected:
	virtual void BeginPlay() override;
	
	// 현재 장착 무기
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> CurrentWeapon;

	// 장착/해제  여부. 적은 항상 손에 듦
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bUseSheathing = false;

	
	// 폴백 — CurrentWeapon 미지정 시(적 등) 사용 - 임시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USkeletalMesh> WeaponMeshAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName AttachSocketName = TEXT("weapon_r");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponComponentTag = TEXT("Weapon");

	// 다중 무기 시 발검 이벤트는 1개 컴포넌트만 발사(총 쪽은 false)
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bBroadcastsToggleEvent = true;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 이동 속도
	UPROPERTY(EditAnywhere, Category = "Weapon|Anim")
	float WalkSpeedThreshold = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Anim")
	float RunSpeedThreshold = 300.f;

private:
	UPROPERTY()
	TObjectPtr<UMeshComponent> WeaponMesh;
	
	void AttachWeaponToSocket(FName SocketName); // 손/등 공용 재부착 (GripPoint 역보정 포함)

	// 전투 시작 시 
	FDelegateHandle InCombatTagHandle;
	void RegisterCombatTagListener();            // ASC 준비되면 InCombat 태그
	
	UFUNCTION()
	void OnInCombatTagChanged(const FGameplayTag Tag, int32 NewCount); 

	UAnimMontage* SelectEquipMontage(const FEquipMontageSet& Set) const;

	// 전투 행동 시 (닷지, 공격, 패링 등)
	FDelegateHandle InActionTagHandle;
	void RegisterInActionTagListener();
	
	UFUNCTION()
	void OnInActionTagChanged(const FGameplayTag Tag, int32 NewCount);
};
