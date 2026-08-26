// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KDAnimNotify_WeaponAttach.generated.h"

/**
 * 
 */

// 장착: 손, 해제: 등
UENUM()
enum class EWeaponAttachTarget : uint8
{
	ToHand,
	ToSheath
};

UCLASS(meta = (DisplayName = "Weapon Attach"))
class PROJECT_KD_API UKDAnimNotify_WeaponAttach : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponAttachTarget Target = EWeaponAttachTarget::ToHand;
	
	// 대상 무기의 WeaponComponentTag. None이면 모든 무기에 적용(동시 발검)
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FName TargetWeaponTag;
};
