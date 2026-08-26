// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "KDPlayerAbilityInputComponent.generated.h"

// 플레이어 입력 -> 어빌리티 활성화 담당

struct FGameplayTag;
class UKDSprintComponent;
class UKDLockOnComponent;
class UKDComboComponent;
class UKDInputBufferComponent;
class UAbilitySystemComponent;

UCLASS( ClassGroup=(KD), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UKDPlayerAbilityInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKDPlayerAbilityInputComponent();
	
	void TryLightAttack() const;
	void TryHeavyAttack() const;
	void TryParry() const;
	void TryParryStop() const;
	void TryDodge() const;
	void TryExecute() const;
	void TryAimStart() const;
	void TryAimStop() const;
	void TryMovementCancel() const;

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 처형 발동 사거리
	UPROPERTY(EditAnywhere, Category = "Action|Execution", meta = (ClampMin = "50.0", ClampMax = "600.0"))
	float ExecutionRange = 250.f;

	// 이동 입력이 끊을 수 있는 어빌리티 태그
	UPROPERTY(EditAnywhere, Category = "Cancel")
	FGameplayTagContainer MovementCancelableTags;

	// 캔슬 윈도우에서 다음 공격이 끊을 수 있는 어빌리티 태그
	UPROPERTY(EditAnywhere, Category = "Cancel")
	FGameplayTagContainer AttackCancelableTags;

private:
	UAbilitySystemComponent* GetASC() const;
	
	// 버퍼에 있으면 꺼내 어빌리티 발동
	void TryConsumeAndActivate(UAbilitySystemComponent* ASC, bool bCanCancel,
		const FGameplayTag& InputTag, const FGameplayTag& AbilityTag) const;
	
	bool ActivateByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
	void CancelByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
	
	// 가던 방향 반대 입력 시 제자리 턴
	void UpdateTurnInPlace(UAbilitySystemComponent* ASC) const;
	
	// 버퍼된 회피 | 공격 꺼내기
	void ConsumeBufferedInput(UAbilitySystemComponent* ASC) const;
	
	UPROPERTY(Transient) TObjectPtr<UKDInputBufferComponent> InputBuffer;
	UPROPERTY(Transient) TObjectPtr<UKDComboComponent> ComboComp;
	UPROPERTY(Transient) TObjectPtr<UKDLockOnComponent> LockOnComp;
	UPROPERTY(Transient) TObjectPtr<UKDSprintComponent> SprintComp;
};
