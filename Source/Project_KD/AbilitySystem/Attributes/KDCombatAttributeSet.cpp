#include "AbilitySystem/Attributes/KDCombatAttributeSet.h"

#include "AbilitySystem/Attributes/KDCharacterAttributeSet.h"
#include "Engine/Engine.h"
#include "GameplayEffectExtension.h"
#include "HAL/IConsoleManager.h"
#include "KDGameplayTags.h"

#if !UE_BUILD_SHIPPING
// 개발용 데미지 표시 스위치 — 콘솔 KD.ShowDamage 1
static TAutoConsoleVariable<int32> CVarShowDamage(
	TEXT("KD.ShowDamage"), 0,
	TEXT("피격 데미지 온스크린 표시 유무"), ECVF_Cheat);
#endif

UKDCombatAttributeSet::UKDCombatAttributeSet()
{
	// 기능 : 전투 어트리뷰트 초기값
	InitAttackPower(20.0f);
	InitDamageReductionRate(0.0f);
	InitShieldAbsorbRate(0.4f);
	InitBlockShieldAbsorbRate(0.8f);
}

void UKDCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 기능 : 비율 0~1 | 흡수율 하한 0
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetDamageReductionRateAttribute()) { NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f); }
	if (Attribute == GetShieldAbsorbRateAttribute()) { NewValue = FMath::Max(NewValue, 0.0f); }
	if (Attribute == GetBlockShieldAbsorbRateAttribute()) { NewValue = FMath::Max(NewValue, 0.0f); }
}

void UKDCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 기능 : 들어온 데미지를 패링 | 비율 경감 | Shield | Health 순으로 분배
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetIncomingDamageAttribute()) { return; }

	// 버킷 비움 — 메타 어트리뷰트는 1회용
	const float LocalDamage = GetIncomingDamage();
	SetIncomingDamage(0.0f);
	if (LocalDamage <= 0.0f) { return; }

	// Health 소유자 = AS_CharacterBase — 공유 ASC 경유
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) { return; }

	// 공격자가 정면 반구(좌우 90도) 안인지
	float HitAngle = 0.f;
	const bool bFrontalAttack = IsFrontalAttack(Data, ASC, HitAngle);

	// 언블록 유무 — Ability.Combat.Unblockable 이면 아래 패링 분기 전부 스킵
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
	const bool bUnblockable = SpecAssetTags.HasTag(GameplayTags::Ability_Combat_Unblockable);

	// 퍼펙트 | 적 방어형 패링이 삼키면 여기서 끝
	if (TryInterceptByParry(Data, ASC, bFrontalAttack, bUnblockable, HitAngle)) { return; }

	// 가드 유무(가드한 상태로 맞았는지)
	const bool bBlockedHit = !bUnblockable && bFrontalAttack && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying);

	// 비율 경감 + Shield 흡수 후 체력으로 갈 몫
	const float ToHealth = ApplyMitigation(ASC, LocalDamage, bBlockedHit);

	// 치명 선판정 — 히트리액션보다 먼저
	const float NewHealth = ASC->GetNumericAttribute(UKDCharacterAttributeSet::GetHealthAttribute()) - ToHealth;
	const bool bLethal = ToHealth > 0.0f && NewHealth <= 0.0f;

	// 히트리액션 신호 — 치명타는 스킵
	if (!bLethal) { SendHitReact(Data, ASC, bBlockedHit); }

	if (ToHealth <= 0.0f) { return; } // 실드가 전부 받았거나 경감 후 0

	// 개발용 데미지 표시 — 값 = 실드·방어 경감 후 체력에 들어간 최종량
#if !UE_BUILD_SHIPPING
	if (GEngine && CVarShowDamage.GetValueOnGameThread() > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
			FString::Printf(TEXT("%s  -%.0f"), *GetNameSafe(ASC->GetAvatarActor()), ToHealth));
	}
#endif

	// Health 차감 — 0 도달 시 HandleDeath 가 이 스택 안에서 완료. 이 아래 사후 로직 X
	// SetNumericAttributeBase = PreAttributeChange 클램프 스킵 — 여기서 하한 0
	ASC->SetNumericAttributeBase(UKDCharacterAttributeSet::GetHealthAttribute(), FMath::Max(NewHealth, 0.0f));
}

bool UKDCombatAttributeSet::IsFrontalAttack(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC, float& OutHitAngle) const
{
	// 기능 : 공격자가 정면 반구(좌우 90도) 안인지 — 기준 = EffectCauser
	// GetInstigator 는 원점이 PlayerState 라 각도 부정확
	OutHitAngle = 0.f;

	const AActor* Defender = ASC->GetAvatarActor();
	if (!Defender) { return false; }

	const AActor* Attacker = Data.EffectSpec.GetContext().GetEffectCauser();
	if (!Attacker) { return false; }

	const FVector ToAttacker = (Attacker->GetActorLocation() - Defender->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward = Defender->GetActorForwardVector().GetSafeNormal2D();

	OutHitAngle = FMath::FindDeltaAngleDegrees(Forward.Rotation().Yaw, ToAttacker.Rotation().Yaw);
	return FVector::DotProduct(Forward, ToAttacker) > 0.f;
}

bool UKDCombatAttributeSet::TryInterceptByParry(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC,
	bool bFrontalAttack, bool bUnblockable, float HitAngle) const
{
	// 기능 : 데미지를 0으로 삼키는 패링 2종 — 퍼펙트 | 적 방어형
	if (bUnblockable || !bFrontalAttack) { return false; }

	// 퍼펙트 패링 — 데미지 0 + 이벤트 발행. 시각 | 슬로우모션 = 리스너 BP
	if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_PerfectParryReady))
	{
		FGameplayEventData EventData;
		EventData.EventTag = GameplayTags::Event_Combat_PerfectParryTriggered;
		EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator();
		EventData.Target = ASC->GetAvatarActor();
		EventData.EventMagnitude = HitAngle;
		ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
		return true;
	}

	// 적 방어형 패링 — 정면 가드 시 데미지 0. 슬로우모션 경로 X
	if (ASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying))
	{
		// 클래시 이벤트 — ContextHandle 의 HitResult = GA_EnemyParry 의 GC 위치
		FGameplayEventData ParryData;
		ParryData.EventTag = GameplayTags::Event_Combat_ParrySuccess;
		ParryData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
		ParryData.Target = ASC->GetAvatarActor();
		ParryData.ContextHandle = Data.EffectSpec.GetContext();
		ASC->HandleGameplayEvent(ParryData.EventTag, &ParryData);
		return true;
	}

	return false;
}

float UKDCombatAttributeSet::ApplyMitigation(UAbilitySystemComponent* ASC, float Damage, bool bBlocked) const
{
	// 기능 : 비율 경감 후 Shield 흡수 — 반환 = 체력으로 갈 몫
	// 비율 경감 — 1 이면 무효. 클램프 = 비율이 1 초과면 데미지가 음수(회복)가 된다
	const float Mitigated = Damage * FMath::Clamp(1.0f - GetDamageReductionRate(), 0.0f, 1.0f);

	// 실드 흡수 비율 — 정면 방어 중이면 실드가 더 받는다
	const float AbsorbRate = bBlocked ? GetBlockShieldAbsorbRate() : GetShieldAbsorbRate();
	const float Shield = ASC->GetNumericAttribute(UKDCharacterAttributeSet::GetShieldAttribute());
	if (Shield <= 0.0f || Mitigated <= 0.0f) { return Mitigated; }

	// 실드가 받는 양 = 피해 x 흡수율 | 남은 실드 중 작은 쪽
	const float Absorbed = FMath::Min(Shield, Mitigated * AbsorbRate);
	ASC->SetNumericAttributeBase(UKDCharacterAttributeSet::GetShieldAttribute(), Shield - Absorbed);
	return FMath::Max(Mitigated - Absorbed, 0.0f);
}

void UKDCombatAttributeSet::SendHitReact(const FGameplayEffectModCallbackData& Data, UAbilitySystemComponent* ASC, bool bBlockedHit) const
{
	// 기능 : 히트리액션 신호
	// 처형 GE = EffectCauser 자기 자신 — 전방판정 false 로 패링 분기 스킵
	FGameplayEventData HitReactData;
	HitReactData.EventTag = GameplayTags::Event_Combat_HitReact;
	HitReactData.Instigator = Data.EffectSpec.GetContext().GetEffectCauser();
	HitReactData.ContextHandle = Data.EffectSpec.GetContext();
	HitReactData.Target = ASC->GetAvatarActor();
	HitReactData.EventMagnitude = bBlockedHit ? 1.0f : 0.0f; // 1 = 방어 중 피격 | 0 = 무방비 피격
	ASC->HandleGameplayEvent(HitReactData.EventTag, &HitReactData);
}
 