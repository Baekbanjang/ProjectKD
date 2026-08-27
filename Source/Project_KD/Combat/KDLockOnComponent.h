// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "KDLockOnComponent.generated.h"

struct FKDTargetFilter;
class UWidgetComponent;
class UKDLockOnConfig;

// 토글 입력으로 가장 가까운 적(IKDTargetable 구현) 자동 선택, 카메라/캐릭터 추적
UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UKDLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKDLockOnComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Controller의 Handle_LockOnToggle에서 호출.
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	AActor* GetLockedTarget() const { return LockedTarget.Get(); }

	// 락온 대상 검색
	AActor* FindBestTarget() const;
	
	// 필터 조건으로 대상 1명 선택
	AActor* FindTargetByFilter(const FKDTargetFilter& Filter) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TObjectPtr<UKDLockOnConfig> Config;

	// 락온 시 Target에 동적 부착, 해제 시 Owner로 복귀 + 숨김.
	UPROPERTY(VisibleAnywhere, Category = "LockOn")
	TObjectPtr<UWidgetComponent> ReticleWidgetComponent;

private:	
	// 활성화 — 타겟 저장, GAS 태그 부여, 마커 부착, 인터페이스 알림.
	void EngageLockOn(AActor* NewTarget);

	// 해제 — 정리, 태그 제거, 마커 숨김, 인터페이스 알림.
	void DisengageLockOn();

	// 현재 타겟 유효성 — 사망/거리 초과/시야 잃음 검사.
	bool IsTargetStillValid() const;

	// 필터 범위 안의 자격 있는 후보 수집 — 중복 제거, 인터페이스, LoS 통과분
	void GatherCandidates(const FKDTargetFilter& Filter, TArray<AActor*>& OutCandidates) const;

	// 반각 기준 벡터
	FVector GetFilterBasis(const FKDTargetFilter& Filter) const;

#if !UE_BUILD_SHIPPING
	// 필터 범위 디버그 표시 — 도형 · 후보 · 선택 대상
	void DrawFilterDebug(const FKDTargetFilter& Filter, const FVector& Basis,
		const TArray<AActor*>& Candidates, const AActor* Chosen) const;
#endif
	
	// 시야 판정 — 나 -> 대상 직선을 월드 지오메트리가 막는지. 후보 검색과 락온 유지가 공용
	bool HasLineOfSightTo(const AActor* Target) const;

	TWeakObjectPtr<AActor> LockedTarget;
	bool bIsLockedOn = false;

	// 조준 시작 시 락온 해제
	FDelegateHandle AimingTagHandle;
	void RegisterAimingTagListener();
	
	UFUNCTION()
	void OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount);
};
