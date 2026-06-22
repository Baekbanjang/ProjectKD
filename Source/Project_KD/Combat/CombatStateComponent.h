// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CombatStateComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatStateComponent();

protected:
	virtual void BeginPlay() override;

	// 컴포넌트 소멸 시 Attacking 태그 구독 해제
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 감지 반경 - 적이 이 안에 들어오면 전투 모드
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DetectionRadius = 1200.f;

	// 적이 다 사라진 뒤 전투 해제까지 대기(초)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float CombatExitDelay = 4.0f;

	// 감지 주기(초)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ScanInterval = 0.3f;

public:	
	void ScanForEnemies();          // 타이머가 주기적으로 호출
	bool HasEnemyInRange() const;  

	FTimerHandle ScanTimerHandle;
	FTimerHandle ExitTimerHandle;
	
	// 공격 시작  순간 즉시 InCombat 켜기용 구독 핸들
	FDelegateHandle AttackingTagHandle;

	// ASC 준비되면 Attacking 태그 구독 등록 
	void RegisterAttackingTagListener();

	UFUNCTION()
	void OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount);
};
