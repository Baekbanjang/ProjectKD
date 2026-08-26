// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Context/KDGameplayEffectContext.h"


const FKDGameplayEffectContext* FKDGameplayEffectContext::Get(const FGameplayEffectContextHandle& Handle)
{
	// 기능 : 핸들이 FKDGameplayEffectContext 일 때만 반환 — 타입 검사 후 캐스팅
	const FGameplayEffectContext* Base = Handle.Get();
	
	if (Base && Base->GetScriptStruct() == FKDGameplayEffectContext::StaticStruct())
	{
		// 형변환 해서 반환
		return static_cast<const FKDGameplayEffectContext*>(Base);
	}

	return nullptr;
}

FKDGameplayEffectContext* FKDGameplayEffectContext::GetMutable(FGameplayEffectContextHandle& Handle)
{
	// 기능 : Get 의 쓰기용  - const 여부
	FGameplayEffectContext* Base = Handle.Get();
	if (Base && Base->GetScriptStruct() == FKDGameplayEffectContext::StaticStruct())
	{
		return static_cast<FKDGameplayEffectContext*>(Base);
	}

	return nullptr;
}

FGameplayEffectContext* FKDGameplayEffectContext::Duplicate() const
{
	// 기능 : 복사본 생성 — GE Spec 복사 시 엔진이 호출
	// 오버라이드 X 면 부모 타입으로 생성돼 PoiseMultiplier 가 조용히 증발
	FKDGameplayEffectContext* NewContext = new FKDGameplayEffectContext(*this);

	// HitResult 는 부모가 TSharedPtr 로 보유 — 위 복사는 포인터만 복사해 원본과 공유
	// bReset = true 로 값을 담은 별도 FHitResult 를 새로 생성
	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}

	return NewContext;
}

bool FKDGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 기능 : 부모 필드 직렬화 후 Poise 추가 필드 이어붙이기
	Super::NetSerialize(Ar, Map, bOutSuccess);

	// 아카이브 스트림 연산자 — Ar 의 방향에 따라 쓰기 | 읽기 자동 분기
	Ar << PoiseMultiplier;
	return true;
}
