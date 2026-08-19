#include "AbilitySystem/Attributes/AS_Combat.h"

#include "AbilitySystem/Attributes/AS_CharacterBase.h"
#include "Engine/Engine.h"
#include "GameplayEffectExtension.h"
#include "KDGameplayTags.h"

UAS_Combat::UAS_Combat()
{
	// 기능 : 전투 어트리뷰트 초기값
	InitAttackPower(20.0f);
	InitDefense(0.0f);
}

void UAS_Combat::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 기능 : Defense 하한 0
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetDefenseAttribute()) { NewValue = FMath::Max(NewValue, 0.0f); }
}

void UAS_Combat::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 기능 : 들어온 데미지를 패링 | Defense | Shield | Health 순으로 분배
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetIncomingDamageAttribute()) { return; }

	// 버킷 비움 — 메타 어트리뷰트는 1회용
	const float LocalDamage = GetIncomingDamage();
	SetIncomingDamage(0.0f);
	if (LocalDamage <= 0.0f) { return; }

	// Health 소유자 = AS_CharacterBase — 공유 ASC 경유
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) { return; }

	// 공격자가 정면 반구(좌우 90도) 안인지 — 기준 = EffectCauser
	// GetInstigator 는 원점이 PlayerState 라 각도 부정확
	bool bFrontalAttack = false;
	float HitAngle = 0.f; // 정면 기준 부호각 — 우 = + | 좌 = -
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

	// 언블록 유무 — Ability.Combat.Unblockable 이면 아래 패링 분기 전부 스킵
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
	const bool bUnblockable = SpecAssetTags.HasTag(GameplayTags::Ability_Combat_Unblockable);

	// 퍼펙트 패링 — 데미지 0 + 이벤트 발행. 시각 | 슬로우모션 = 리스너 BP
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

	// 적 방어형 패링 — 정면 가드 시 데미지 0. 슬로우모션 경로 X
	if (!bUnblockable && bFrontalAttack
		&& ASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying))
	{
		// 클래시 이벤트 — ContextHandle 의 HitResult = GA_EnemyParry 의 GC 위치
		FGameplayEventData ParryData;
		ParryData.EventTag = GameplayTags::Event_Combat_ParrySuccess;
		ParryData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
		ParryData.Target = ASC->GetAvatarActor();
		ParryData.ContextHandle = Data.EffectSpec.GetContext();
		ASC->HandleGameplayEvent(ParryData.EventTag, &ParryData);
		return;
	}

	// 일반 패링 — 50% 감소
	const bool bBlockedHit = !bUnblockable && bFrontalAttack && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying);
	float FinalDamage = bBlockedHit ? LocalDamage * 0.5f : LocalDamage;

	// Defense 경감 — 하한 0
	const float Mitigated = FMath::Max(FinalDamage - GetDefense(), 0.0f);

	// 실드 경감 — 피해의 이 비율만큼 실드가 대신 받음
	constexpr float ShieldDamageReduction = 0.4f;

	// 체력으로 갈 몫 — 실드가 받은 만큼 줄어듦
	float ToHealth = Mitigated;
	const float Shield = ASC->GetNumericAttribute(UAS_CharacterBase::GetShieldAttribute());
	if (Shield > 0.0f && Mitigated > 0.0f)
	{
		// 실드가 받는 양 = 피해 x 0.4 | 남은 실드 중 작은 쪽
		const float Absorbed = FMath::Min(Shield, Mitigated * ShieldDamageReduction);
		ASC->SetNumericAttributeBase(UAS_CharacterBase::GetShieldAttribute(), Shield - Absorbed);
		ToHealth = Mitigated - Absorbed;
	}

	// 치명 선판정 — 히트리액션보다 먼저
	const float NewHealth = ASC->GetNumericAttribute(UAS_CharacterBase::GetHealthAttribute()) - ToHealth;
	const bool bLethal = ToHealth > 0.0f && NewHealth <= 0.0f;

	// 히트리액션 신호 — 치명타는 스킵
	// 처형 GE = EffectCauser 자기 자신 — 전방판정 false 로 패링 분기 스킵
	if (!bLethal)
	{
		FGameplayEventData HitReactData;
		HitReactData.EventTag = GameplayTags::Event_Combat_HitReact;
		HitReactData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
		HitReactData.ContextHandle = Data.EffectSpec.GetContext();
		HitReactData.Target = ASC->GetAvatarActor();
		HitReactData.EventMagnitude = bBlockedHit ? 1.0f : 0.0f; // 1 = 방어 중 피격 | 0 = 무방비 피격
		ASC->HandleGameplayEvent(HitReactData.EventTag, &HitReactData);
	}

	if (ToHealth <= 0.0f) { return; } // 실드가 전부 받았거나 경감 후 0

	// 개발용 데미지 표시 — 값 = 실드·방어 경감 후 체력에 들어간 최종량. 출시 전 삭제
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
			FString::Printf(TEXT("%s  -%.0f"), *GetNameSafe(ASC->GetAvatarActor()), ToHealth));
	}
#endif

	// Health 차감 — 0 도달 시 HandleDeath 가 이 스택 안에서 완료. 이 아래 사후 로직 X
	// SetNumericAttributeBase = PreAttributeChange 클램프 스킵 — 여기서 하한 0
	ASC->SetNumericAttributeBase(UAS_CharacterBase::GetHealthAttribute(), FMath::Max(NewHealth, 0.0f));
}
 