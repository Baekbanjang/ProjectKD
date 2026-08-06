// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class UWidgetComponent;
class ULockOnConfig;
// 토글 입력으로 가장 가까운 적(IKDTargetable 구현) 자동 선택, 카메라/캐릭터 추적
UCLASS( ClassGroup=(Combat), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULockOnComponent();

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

	const ULockOnConfig* GetConfig() const { return Config; }

	// 후보 적 검색 — Sphere Trace + 시야 콘(Dot) + LoS + IKDTargetable 모두 통과한 가장 가까운 적.
	AActor* FindBestTarget() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TObjectPtr<ULockOnConfig> Config;

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

	TWeakObjectPtr<AActor> LockedTarget;
	bool bIsLockedOn = false;

	// 조준 시작 시 락온 해제
	FDelegateHandle AimingTagHandle;
	void RegisterAimingTagListener();
	
	UFUNCTION()
	void OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount);
};
