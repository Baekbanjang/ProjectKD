// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_KD_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()


public:
	// 무기 스켈레탈 메시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USkeletalMesh> WeaponMesh;

	// 손에 쥘 소켓 (캐릭터 메시 소켓명)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HandSocketName = TEXT("weapon_r");

	// 비전투 시 등/허리에 멜 소켓 (스켈레톤에 추가 필요)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName SheathSocketName = TEXT("weapon_back");

	// 장착 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	TObjectPtr<UAnimMontage> DrawMontage;

	// 해제 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	TObjectPtr<UAnimMontage> SheathMontage;
};
