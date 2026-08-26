// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KDKnockbackComponent.generated.h"


struct FGameplayEventData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKnockbackBegin); // 넉백시 적 Pawn을 정지하기 위한 델리게이트

UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UKDKnockbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKDKnockbackComponent();
	
	UPROPERTY(BlueprintAssignable, Category="Knockback")
	FOnKnockbackBegin OnKnockbackBegin;

	void ApplyKnockback(const FGameplayEventData& Payload, float BaseDistance);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Knockback", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float KnockbackDuration = 0.2f;
};
