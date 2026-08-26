#include "Player/KDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDCombatAttributeSet.h"
#include "AbilitySystem/Attributes/KDPlayerAttributeSet.h"
#include "Abilities/GameplayAbility.h"

AKDPlayerState::AKDPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	PlayerAttributes = CreateDefaultSubobject<UKDPlayerAttributeSet>(TEXT("PlayerAttributes"));
	CombatAttributes = CreateDefaultSubobject<UKDCombatAttributeSet>(TEXT("CombatAttributes"));

	// Standard GAS setup — Mixed mode for player-owned ASC
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Higher tick rate so GAS attribute changes replicate promptly when going multiplayer
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AKDPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKDPlayerState::GrantStartupAbilities() 
{
	if (bAbilitiesGranted) return;
	if (!IsValid(AbilitySystemComponent)) return;

	// Single-player demo: granting on owning side is sufficient. Multiplayer migration
	// would gate this on HasAuthority() per CLAUDE.md replication note.
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (!AbilityClass) continue;
		FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
		AbilitySystemComponent->GiveAbility(Spec);
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : StartupEffects)
	{
		if (!EffectClass) continue;
		// 컨텍스트 생성
		FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
		Ctx.AddSourceObject(this); // PlayerState 소스 오브젝트에 추가
		// 1.0f = 레벨
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Ctx);
		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()); // 적용
		}
	}
	
	bAbilitiesGranted = true;
}
