// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Combo/KDComboComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDComboTreeDataAsset.h"

UKDComboComponent::UKDComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const FComboNode* UKDComboComponent::ProcessInput(FGameplayTag InputTag, EComboContext Context)
{
	// 지상 <-> 공중 바뀌면 콤보 중단
	if (Context != LastContext)
	{
		CurrentNodeId = NAME_None;
		ComboDepth = 0;
		LastContext = Context;
	}
	
	const UKDComboTreeDataAsset* Tree = (Context == EComboContext::Air) ? AirComboTree : ComboTree;
	if (!IsValid(Tree))
	{
		UE_LOG(LogTemp, Warning, TEXT("[KD] UComboComponent: ComboTree(ctx=%d) not assigned on %s"),
			(int32)Context, *GetOwner()->GetName());
		return nullptr;
	}

	const FComboNode* Next = nullptr;
	bool bNewCombo = false;
	// 1) 콤보 중이면 현재 노드에서 다음 노드 확인
	if (const FComboNode* Current = Tree->FindNode(CurrentNodeId))
	{
		for (const FComboLink& Link : Current->NextLinks) // 링크 들 확인
		{
			if (Link.InputTag == InputTag)
			{
				Next = Tree->FindNode(Link.NextNodeId); // 해당 이름으로 노드 찾기
				break;
			}
		}
	}
	
	// 2) 콤보 중 아니거나 다음 노드 없으면 시작 노드 확인
	if (!Next)
	{
		Next = FindEntryNode(Tree, InputTag);
		bNewCombo = (Next != nullptr);
	}
	
	// 콤보로 연결 유무 확인
	if (bNewCombo)
	{
		ComboDepth = 0;
	}
	++ComboDepth;
	CurrentNodeId = Next ? Next->NodeId : NAME_None;

	// 리셋 타이머 재시작 — 노드 결정 후에 InputWindow를 사용
	if (UWorld* World = GetWorld())
	{
		// 노드별 값 우선, 0이면 컴포넌트 공용값
		const float Window = (Next && Next->InputWindow > 0.f) ? Next->InputWindow : ComboResetTime;
		World->GetTimerManager().SetTimer(ResetTimerHandle, this,
			&UKDComboComponent::OnResetTimeout, Window, false);
	}

	return Next;
}

void UKDComboComponent::ClearHistory()
{
	CurrentNodeId = NAME_None;
	ComboDepth = 0;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetTimerHandle);
	}
}

void UKDComboComponent::EnterNode(FName NodeId, float ResetTimeOverride)
{
	// 현재 위치를 이 노드로, 콤보는 처음부터
	CurrentNodeId = NodeId;
	ComboDepth = 0;

	// 노드의 InputWindow 조회
	const FComboNode* Node = ComboTree ? ComboTree->FindNode(NodeId) : nullptr;

	if (!Node && AirComboTree)
	{
		Node = AirComboTree->FindNode(NodeId);
	}

	// 인자 안 주면 기본값 사용
	float Time = ComboResetTime;

	// 인자 > 노드 InputWindow > 공용값
	if (ResetTimeOverride > 0.f)
	{
		Time = ResetTimeOverride;
	}
	else if (Node && Node->InputWindow > 0.f)
	{
		Time = Node->InputWindow;
	}

	// 이 시간 안에 입력 없으면 콤보 중단
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ResetTimerHandle, this,
			&UKDComboComponent::OnResetTimeout, Time, false);
	}
}

void UKDComboComponent::EnterEvadeNode(bool bPerfect)
{
	// 회피 합류 — 트리 DA의 진입 ID로 EnterNode 호출, 트리 없으면 기존 이름 유지
	const UKDComboTreeDataAsset* Tree = ComboTree ? ComboTree : AirComboTree;
	const FName NodeId = bPerfect
		? (Tree ? Tree->JustEvadeEntryId : TEXT("JustEvade"))
		: (Tree ? Tree->EvadeEntryId : TEXT("Evade"));

	// 0.8f 고정값 — 추후 InputWindow로 흡수 예정
	EnterNode(NodeId, 0.8f);
}

const FComboNode* UKDComboComponent::FindEntryNode(const UKDComboTreeDataAsset* Tree, FGameplayTag InputTag) const
{
	const UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	
	// 위에서부터 첫 매칭 채택 — 조건 좁은 것(퍼펙트회피)이 위에 있어야 함
	for (const FComboEntry& Entry : Tree->Entries)
	{
		if (Entry.InputTag != InputTag) continue;
		
		// 태그 비면 조건 없음(평상시), 있으면 ASC가 그 태그 갖고 있어야 통과
		if (Entry.RequiredStateTag.IsValid())
		{
			if (!ASC || !ASC->HasMatchingGameplayTag(Entry.RequiredStateTag)) continue;
		}
		
		if (const FComboNode* Node = Tree->FindNode(Entry.StartNodeId))
		{
			return Node;
		}
	}
	
	return nullptr;
}

void UKDComboComponent::OnResetTimeout()
{
	ClearHistory();
}

