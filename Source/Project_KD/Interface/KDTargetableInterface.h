// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KDTargetableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UKDTargetableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 락온 시스템이 후보 적을 판단할 때 사용하는 인터페이스.
 * 적 클래스(KDEnemyBaseCharacter)에 구현하면 락온 시스템이 Cast 없이 검사 가능.
 */
class PROJECT_KD_API IKDTargetableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 락온 가능 상태인지
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	bool CanBeTargeted() const;
	virtual bool CanBeTargeted_Implementation() const { return true; }

	// 락온 당했을 때 호출(적)
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	void OnTargeted(bool bIsTargeted);
	virtual void OnTargeted_Implementation(bool bIsTargeted) {}

	// 락온 조준점
	UFUNCTION(BlueprintNativeEvent, Category = "LockOn")
	FVector GetLockOnPoint() const;
	virtual FVector GetLockOnPoint_Implementation() const {return FVector::ZeroVector;}
};
