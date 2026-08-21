// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerAttackBase.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "AbilitySystem/Combo/ComboTreeDataAsset.h"


void UGA_PlayerAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 콤보 컴포넌트가 없으면(= 플레이어가 아니면) 노드도 없음 -> 몽타주 없이 종료
	UComboComponent* Combo = GetComboComponentFromActorInfo();

	const FComboNode* Node = IsValid(Combo)
		? Combo->ProcessInput(ComboInputTag)
		: nullptr;

	// 매 시작에 디폴트 복원 — 직전 값이 다음 활성화까지 남는 것 막음
	DamageEffectClass = DefaultDamageEffectClass;
	DamageMultiplier = DefaultDamageMultiplier;
	KnockbackMultiplier = DefaultKnockbackMultiplier;

	if (Node)
	{
		// 노드 = 이번 콤보
		AttackMontage = IsValid(Node->Montage) ? Node->Montage : nullptr; // 노드의 몽타주 GA 변수에 대입
		if (!AttackMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[KD] Combo node '%s' 몽타주 미지정"), *Node->NodeId.ToString());
		}
		
		if (Node->DamageEffectClass)
		{
			DamageEffectClass = Node->DamageEffectClass;
		}
		if (Node->DamageMultiplier > 0.f)
		{
			DamageMultiplier = Node->DamageMultiplier;
		}
		if (Node->KnockbackMultiplier > 0.f)
		{
			KnockbackMultiplier = Node->KnockbackMultiplier;
		}
	}
	else
	{
		// 트리에서 못 찾음 = 데이터 문제, 몽타주 없이 두면 부모가 EndAbility
		AttackMontage = nullptr;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
