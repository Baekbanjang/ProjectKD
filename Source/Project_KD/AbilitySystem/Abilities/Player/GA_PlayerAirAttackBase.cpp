// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerAirAttackBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "AbilitySystem/Combo/ComboTreeDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGA_PlayerAirAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	// 매 활성화마다 막타 플래그 리셋 (InstancedPerActor 잔류 차단).
	bIsFinisher = false;

	// 콤보 컴포넌트 없으면 몽타주 없이 종료
	UComboComponent* Combo = GetComboComponentFromActorInfo();

	// 공중 컨텍스트 -> AirComboTree만 봄
	const FComboNode* Node = IsValid(Combo)
		? Combo->ProcessInput(ComboInputTag, EComboContext::Air)
		: nullptr;
	// 매 시작에 디폴트 복원 — 직전 노드 값 잔류 차단
	DamageEffectClass = DefaultAirDamageEffectClass;
	DamageMultiplier = DefaultAirDamageMultiplier;
	
	if (Node)
	{
		AttackMontage = IsValid(Node->Montage) ? Node->Montage : nullptr; // 노드의 몽타주 GA 변수에 대입
		if (!AttackMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("[KD] Air combo node '%s' 몽타주 미지정"), *Node->NodeId.ToString());
		}

		if (Node->DamageEffectClass)
		{
			DamageEffectClass = Node->DamageEffectClass;
		}

		if (Node->DamageMultiplier > 0.f)
		{
			DamageMultiplier = Node->DamageMultiplier;
		}
		
		// 다음 없는 노드 = 막타(피니셔)
		bIsFinisher = (Node->NextLinks.Num() == 0);
	}
	else
	{
		// 트리에서 못 찾음 = 데이터 문제, 몽타주 없이 두면 부모가 EndAbility
		AttackMontage = nullptr;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UGA_PlayerAirAttackBase::OnActivated()
{
	Super::OnActivated(); // 락온 자동 조준 유지
	
	ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* Move = Char ? Char->GetCharacterMovement() : nullptr;
	if (!Move) return;

	// 억제 전 GravityScale 캐시 (InstancedPerActor라 매 활성화마다 갱신).
	OrigGravityScale = Move->GravityScale;
	Move->GravityScale = 0.f;   // 중력 억제 = 체공
	Move->Velocity.Z = 0.f;     // 그 높이에 고정 (관성 제거)
}

void UGA_PlayerAirAttackBase::OnCleanup(bool bWasCancelled)
{
	// 모든 종료 경로에서 중력 복원 (완료/피격중단/타임아웃/외부캔슬 전부)
	if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->GravityScale = OrigGravityScale;
		}
	}

	// 막타에서만 쿨다운 부여 -> 떨어지고 잠깐 막힘 -> 무한 체공 차단.
	// (bWasCancelled 무관 — 막타는 캔슬로 끝나도 콤보 끝이므로 쿨다운.)
	if (bIsFinisher && AirComboLockGE)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AirComboLockGE, 1.f, Ctx);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	Super::OnCleanup(bWasCancelled);
}
