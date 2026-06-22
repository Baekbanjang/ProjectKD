// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComboTreeDataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ComboComponent.generated.h"


struct FComboBranch;

// GA가 ProcessInput 호출 → 정확 매칭 시 분기 포인터 반환, 미매칭/Prefix 진행 중 시 nullptr.
// ResetTime 안에 다음 입력 없으면 InputHistory 자동 초기화. 정확 매칭 직후도 자동 리셋 (콤보 완료).
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UComboComponent();

	// GA에서 호출 (C++ 전용). InputTag 누적 + 리셋 Timer 재시작 + 정확 매칭 검사.
	// 반환: 정확 매칭 시 분기 포인터 (GA가 AttackMontage 교체), nullptr 시 GA는 부모 디폴트 재생.
	const FComboBranch* ProcessInput(FGameplayTag InputTag, EComboContext Context = EComboContext::Ground);

	// GA에서 미매칭 시 현재 콤보 진행 길이 조회 → 디폴트 Montage 배열 인덱싱용 (1타째=1).
	UFUNCTION(BlueprintCallable, Category = "Combo")
	int32 GetInputHistoryLength() const { return InputHistory.Num(); }

	// 입력 배수(1.0). GA가 PlayRate에 곱함.
	UFUNCTION(BlueprintCallable, Category = "Combo")
	float GetTempoMultiplier() const { return CurrentTempoMultiplier; }

	// 외부 강제 리셋 (예: 피격 시 콤보 끊김 — M2 F11 적 피격 리액션과 연동).
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ClearHistory();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UComboTreeDataAsset> ComboTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UComboTreeDataAsset> AirComboTree;

	// 마지막 입력 후 N초 미입력 시 InputHistory 자동 초기화.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ComboResetTime = 1.5f;

	// X=입력 간격(초), Y=배속 배수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Tempo")
	TObjectPtr<UCurveFloat> TempoCurve;

	// 몽타주 배속 제한
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Tempo", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float MaxTempoMultiplier = 1.3f;

private:
	// 입력 시퀀스 히스토리. UPROPERTY로 표시해 GC 안전.
	UPROPERTY()
	TArray<FGameplayTag> InputHistory;

	FTimerHandle ResetTimerHandle;

	// 직전 입력 시각(초). 첫 입력 판별용 음수 초기값.
	float LastInputTime = -1.f;

	// 직전 ProcessInput 컨텍스트 — 지상 - 공중 전환 감지용.
	EComboContext LastContext = EComboContext::Ground;

	// 현재 배속 배수.
	float CurrentTempoMultiplier = 1.f;
	
	void OnResetTimeout();
};
