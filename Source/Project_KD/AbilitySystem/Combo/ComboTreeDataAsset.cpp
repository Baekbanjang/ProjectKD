// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Combo/ComboTreeDataAsset.h"

const FComboNode* UComboTreeDataAsset::FindNode(FName NodeId) const
{
	// 이름 비었으면 찾을 대상 없음
	if (NodeId.IsNone()) return nullptr;

	// Nodes 배열에서 이름 같은 항목 검색
	for (const FComboNode& Node : Nodes)
	{
		if (Node.NodeId == NodeId)
		{
			// 복사본 아닌 원본 주소 반환
			return &Node;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[KD] ComboTree: '%s' 노드 없음"), *NodeId.ToString());
	return nullptr;
}
