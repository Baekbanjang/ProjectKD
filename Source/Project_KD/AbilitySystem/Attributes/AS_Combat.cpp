#include "AbilitySystem/Attributes/AS_Combat.h"

#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "GameplayEffectExtension.h"
#include "KDGameplayTags.h"

UAS_Combat::UAS_Combat()
{
	InitAttackPower(20.0f);
	InitDefense(0.0f);
}

void UAS_Combat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 음수 방지
	if (Attribute == GetDefenseAttribute()) { NewValue = FMath::Max(NewValue, 0.0f); }
}

void UAS_Combat::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetIncomingDamageAttribute()) { return; }

	// 게이트웨이 비움 — 메타 어트리뷰트는 1회용 버킷(영속 X).
	const float LocalDamage = GetIncomingDamage();
	SetIncomingDamage(0.0f);
	if (LocalDamage <= 0.0f) { return; }

	// Health는 AS_CharacterBase 소유 — 공유 ASC로 적용.
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) { return; }

	// 전방 판정: 공격자가 정면 반구(±90°) 안일 때만 방어/패링 성립.
	// EffectCauser = 공격 아바타 사용(GetInstigator()는 PlayerState 원점이라 각도 부정확).
	bool bFrontalAttack = false;
	float HitAngle = 0.f; // 정면 기준 좌우 부호각(+우 / -좌)
	if (const AActor* Defender = ASC->GetAvatarActor())
	{
		if (const AActor* Attacker = Data.EffectSpec.GetContext().GetEffectCauser())
		{
			const FVector ToAttacker = (Attacker->GetActorLocation() - Defender->GetActorLocation()).GetSafeNormal2D();
			const FVector Forward = Defender->GetActorForwardVector().GetSafeNormal2D();
			bFrontalAttack = FVector::DotProduct(Forward, ToAttacker) > 0.f;
			HitAngle = FMath::FindDeltaAngleDegrees(Forward.Rotation().Yaw, ToAttacker.Rotation().Yaw);
		}
	}

	// 언블록(노랑 전조) — Ability.Combat.Unblockable asset tag를 단 공격은 아래 패링 분기 전부 스킵(회피만 가능).
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
	const bool bUnblockable = SpecAssetTags.HasTag(GameplayTags::Ability_Combat_Unblockable);

	// Perfect Parry — 데미지 0 + GameplayEvent 발행 (시각/슬로우모션은 listener BP가 처리).
	if (!bUnblockable && bFrontalAttack && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_PerfectParryReady))
	{
		FGameplayEventData EventData;
		EventData.EventTag = GameplayTags::Event_Combat_PerfectParryTriggered;
		EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator();
		EventData.Target = ASC->GetAvatarActor();
		EventData.EventMagnitude = HitAngle;
		ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
		return;
	}

	// 적 방어형 패링 — 정면 가드 시 데미지 완전 차단(0 = 무효화). 슬로모 경로를 안 타 플레이어는 무경직(비대칭).
	if (!bUnblockable && bFrontalAttack
		&& ASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying))
	{
		// 막아낸 순간 클래시 이벤트. ContextHandle의 HitResult로 리스너(GA_EnemyParry)가 GC 위치를 잡음.
		FGameplayEventData ParryData;
		ParryData.EventTag = GameplayTags::Event_Combat_ParrySuccess;
		ParryData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
		ParryData.Target = ASC->GetAvatarActor();
		ParryData.ContextHandle = Data.EffectSpec.GetContext();
		ASC->HandleGameplayEvent(ParryData.EventTag, &ParryData);
		return;
	}

	// 일반 Parry — 50% 감소.
	const bool bBlockedHit = !bUnblockable && bFrontalAttack && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying);
	float FinalDamage = bBlockedHit ? LocalDamage * 0.5f : LocalDamage;

	// Defense 경감 + 치명 선판정 — 히트리액션보다 먼저. 치명타에 움찔 이벤트가 나가면 같은 프레임 사망 연출에
	// 끊겨 몽타주 순서가 꼬임.
	const float Mitigated = FMath::Max(FinalDamage - GetDefense(), 0.0f);
	const float NewHealth = ASC->GetNumericAttribute(UAS_CharacterBase::GetHealthAttribute()) - Mitigated;
	const bool bLethal = Mitigated > 0.0f && NewHealth <= 0.0f;

	// 히트리액션 신호 — 리스너 BP가 EventMagnitude로 방어/무방비 몽타주 선택. 치명타는 스킵(사망 연출 소유).
	// 처형 GE(self-apply)는 EffectCauser=자기 자신 → 전방판정 false → 패링 분기 자연 스킵.
	if (!bLethal)
	{
		FGameplayEventData HitReactData;
		HitReactData.EventTag = GameplayTags::Event_Combat_HitReact;
		HitReactData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
		HitReactData.ContextHandle = Data.EffectSpec.GetContext();
		HitReactData.Target = ASC->GetAvatarActor();
		HitReactData.EventMagnitude = bBlockedHit ? 1.0f : 0.0f; // 1=방어 중 맞음, 0=그냥 맞음
		ASC->HandleGameplayEvent(HitReactData.EventTag, &HitReactData);
	}

	if (Mitigated <= 0.0f) { return; } // 경감 후 0 이하면 적용 불필요

	// Health 차감 — 이 Set은 Health 델리게이트를 동기 발화한다. 0 도달 시 HandleDeath가
	// 이 호출 스택 안에서 완료됨 → 아래에 사후 로직 추가 금지.
	// SetNumericAttributeBase는 PreAttributeChange 클램프를 건너뛰므로 여기서 0 하한.
	ASC->SetNumericAttributeBase(UAS_CharacterBase::GetHealthAttribute(), FMath::Max(NewHealth, 0.0f));
}
 