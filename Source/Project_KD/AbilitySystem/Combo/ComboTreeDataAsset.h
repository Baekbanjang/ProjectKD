// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ComboTreeDataAsset.generated.h"

/*
* 콤보 = 노드 지도, 노드 하나 = 공격 한 타
* 노드끼리는 배열 인덱스 대신 NodeId(이름)로 연결 — 순서 바꿔도 안 깨지게
*/

class UAnimMontage;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EComboContext : uint8
{
	Ground = 0  UMETA(DisplayName = "Ground"),
	Air    = 1  UMETA(DisplayName = "Air"),
};

// 노드에서 나가는 길 하나 — 이 입력이 오면 다음 노드로
USTRUCT(BlueprintType)
struct FComboLink
{
	GENERATED_BODY()
	
	// Input.Combo.Light / Input.Combo.Heavy
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTag InputTag;
	
	// 갈 노드 NodeId
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName NextNodeId;
};

// 공격 하나 = 노드 하나
USTRUCT(BlueprintType)
struct FComboNode
{
	GENERATED_BODY()
	
	// 이름(C01_2) — 링크가 이 이름으로 확인
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName NodeId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage;
	
	// 다음 갈 수 있는 공격 — 없으면 마지막 공격(피니셔)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FComboLink> NextLinks;
	
	// 비우면 GA 기본 GE 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float InputWindow = 0.f;

	// 데미지 배수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DamageMultiplier = 0.f;

	// 넉백 배수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float KnockbackMultiplier = 0.f;
};

// 콤보 시작 — 상황 + 입력 -> 첫 노드
USTRUCT(BlueprintType)
struct FComboEntry
{
	GENERATED_BODY()
	
	// ASC가 이 태그 갖고 있어야 열림, 비우면 조건 없음(평상시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTag RequiredStateTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTag InputTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName StartNodeId;
};


UCLASS(BlueprintType)
class PROJECT_KD_API UComboTreeDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	// 위에서부터 첫 매칭 채택 — 조건 좁은 것(퍼펙트회피)을 위에
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FComboEntry> Entries;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FComboNode> Nodes;
	
	// NodeId로 노드 찾기, 없으면 nullptr
	const FComboNode* FindNode(FName NodeId) const;
};
