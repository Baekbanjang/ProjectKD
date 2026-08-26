// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KDWeaponDataAsset.generated.h"

class UStaticMesh;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FEquipMontageSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	TObjectPtr<UAnimMontage> Idle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	TObjectPtr<UAnimMontage> Walk;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	TObjectPtr<UAnimMontage> Run;
};
/**
 *
 */
UCLASS()
class PROJECT_KD_API UKDWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()


public:
	// 무기 스켈레탈 메시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USkeletalMesh> WeaponMesh;

	// 스태틱메시 무기(총/검 팩 에셋)용 — WeaponMesh(스켈레탈)와 둘 중 하나만 지정
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<UStaticMesh> WeaponStaticMesh;

	// 손에 쥘 소켓 (캐릭터 메시 소켓명)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HandSocketName = TEXT("weapon_r");

	// 비전투 시 등/허리에 멜 소켓 (스켈레톤에 추가 필요)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName SheathSocketName = TEXT("weapon_back");


	// 각 속도별 
	// 장착 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	FEquipMontageSet DrawMontages;

	// 해제 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Anim")
	FEquipMontageSet SheathMontages;
};
