// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerAirAttackBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Combo/ComboComponent.h"
#include "AbilitySystem/Combo/ComboTreeDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/KDPlayerCharacter.h"

void UGA_PlayerAirAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	// 매 활성화마다 막타 플래그 리셋 (InstancedPerActor 잔류 차단).
	bIsFinisher = false;

	// 적이 이 GA 쓰면 Cast 실패 -> Combo == nullptr -> 디폴트 동작.
	AKDPlayerCharacter* Player = Cast<AKDPlayerCharacter>(GetAvatarActorFromActorInfo());
	UComboComponent* Combo = IsValid(Player) ? Player->GetComboComponent() : nullptr;

	// 공중 컨텍스트로 콤보 입력 처리 (지상 트리 X, AirComboTree만 봄).
	const FComboBranch* Matched = IsValid(Combo)
		? Combo->ProcessInput(ComboInputTag, EComboContext::Air)
		: nullptr;

	// 매 시작에 디폴트 복원 — 직전 분기 값 잔류 차단.
	DamageEffectClass = DefaultAirDamageEffectClass;

	if (Matched)
	{
		// 정확 매칭 = 완성 시퀀스 = 막타. 분기 Montage/GE로 교체.
		AttackMontage = IsValid(Matched->Montage) ? Matched->Montage : nullptr;
		if (Matched->DamageEffectClass)
		{
			DamageEffectClass = Matched->DamageEffectClass;
		}
		bIsFinisher = true;
	}
	else
	{
		// 미매칭 = 콤보 진행 중. 길이로 디폴트 배열 인덱싱 (cap, wrap 금지).
		const int32 Num = DefaultAirMontages.Num();
		if (Num > 0)
		{
			const int32 Length = FMath::Max(IsValid(Combo) ? Combo->GetInputHistoryLength() : 1, 1);
			const int32 Idx = FMath::Min(Length - 1, Num - 1); // 4타 도달 시 멈춤 = 콤보 끝
			AttackMontage = DefaultAirMontages[Idx];
			if (Length >= Num)
			{
				bIsFinisher = true; // 배열 끝 도달 = 막타
			}
		}
		else
		{
			// 배열 비어있으면 부모가 EndAbility
			AttackMontage = nullptr;
		}
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
