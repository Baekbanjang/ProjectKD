// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "KDGameplayEffectContext.generated.h"  

/**
 * 
 */
USTRUCT()
struct PROJECT_KD_API FKDGameplayEffectContext: public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	// Poise 차감 배수
	UPROPERTY()
	float PoiseMultiplier = 0.f;

	// 타입 조회
	static const FKDGameplayEffectContext* Get(const FGameplayEffectContextHandle& Handle);
	static FKDGameplayEffectContext* GetMutable(FGameplayEffectContextHandle& Handle);
	virtual UScriptStruct* GetScriptStruct() const override{return StaticStruct(); }

	// 스펙 복사 시 값 유지
	virtual FGameplayEffectContext* Duplicate() const override;
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};

// 해당 구조체가 어떤 기능을 직접 구현했는지 엔진에 알리는 템플릿
template<>
struct TStructOpsTypeTraits<FKDGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FKDGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
