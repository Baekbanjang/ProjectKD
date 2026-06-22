#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class UWeaponDataAsset;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Weapon), meta=(BlueprintSpawnableComponent))
class PROJECT_KD_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	void AttachWeaponToSocket(FName SocketName);                 // 손/등 공용 재부착 (GripPoint 역보정 포함)
	void RegisterCombatTagListener();                            // ASC 준비되면 InCombat 태그
	
	UFUNCTION()
	void OnInCombatTagChanged(const FGameplayTag Tag, int32 NewCount); 
	
	FDelegateHandle InCombatTagHandle;
};
