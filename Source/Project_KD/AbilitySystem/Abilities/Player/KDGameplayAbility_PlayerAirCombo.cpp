// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/KDGameplayAbility_PlayerAirCombo.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Combo/KDComboTreeDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UKDGameplayAbility_PlayerAirCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	// 매 활성화마다 막타 플래그 리셋 (InstancedPerActor 잔류 차단).
	bIsFinisher = false;

	// 공중 컨텍스트 -> AirComboTree만 봄
	const FComboNode* Node = ApplyComboNode(ComboInputTag, EComboContext::Air,
		DefaultAirDamageEffectClass, DefaultAirDamageMultiplier, DefaultAirKnockbackMultiplier, DefaultAirPoiseMultiplier);

	// 다음 없는 노드 = 막타(피니셔)
	bIsFinisher = (Node && Node->NextLinks.Num() == 0);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UKDGameplayAbility_PlayerAirCombo::OnActivated()
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

void UKDGameplayAbility_PlayerAirCombo::OnCleanup(bool bWasCancelled)
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
