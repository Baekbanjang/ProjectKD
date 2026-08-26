// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KDComboTreeDataAsset.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "KDComboComponent.generated.h"


struct FComboNode;
// GA가 ProcessInput 호출 -> 갈 수 있으면 그 노드 반환, 못 가면 nullptr(GA는 디폴트 몽타주)
// ComboResetTime 안에 다음 입력 없으면 위치 지워짐 = 콤보 끊김

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_KD_API UKDComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKDComboComponent();

	// GA에서 호출 (C++ 전용)
	// 콤보 중이면 현재 노드 NextLinks에서, 아니면 Entries에서 갈 곳 확인
	const FComboNode* ProcessInput(FGameplayTag InputTag, EComboContext Context = EComboContext::Ground);
	
	// 외부 강제 리셋(피격으로 콤보 끊김 등), 이름 유지 = BP 호출부 보호
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ClearHistory();
	
	// 콤보 밖 행동(회피)이 그래프에 합류 — 현재 위치를 이 노드로 이동
	// ResetTimeOverride > 0 이면 그 시간 뒤 만료
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void EnterNode(FName NodeId, float ResetTimeOverride = -1.f);

	// 회피(퍼펙트 포함) 합류 — 트리 DA의 진입 ID로 EnterNode 호출
	void EnterEvadeNode(bool bPerfect);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UKDComboTreeDataAsset> ComboTree;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UKDComboTreeDataAsset> AirComboTree;
	
	// 마지막 입력 후 N초 미입력 시 콤보 중단
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ComboResetTime = 1.5f;
private:
	// 현재 노드
	FName CurrentNodeId = NAME_None;
	
	// 현재 콤보에서 몇 번째 콤보
	int32 ComboDepth = 0;
	
	// 지상 <-> 공중 전환 감지용
	EComboContext LastContext = EComboContext::Ground;
	FTimerHandle ResetTimerHandle;
	
	// 상황 태그 확인 후 시작 노드 선택
	const FComboNode* FindEntryNode(const UKDComboTreeDataAsset* Tree, FGameplayTag InputTag) const;
	void OnResetTimeout();
};
