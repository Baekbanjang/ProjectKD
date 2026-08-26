#include "AbilitySystem/Effects/KDGameplayEffect_Damage_Unblockable.h"

#include "KDGameplayTags.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UKDGameplayEffect_Damage_Unblockable::UKDGameplayEffect_Damage_Unblockable()
{
	// 부모(GE_Damage_Physical)의 데미지 modifier + Invulnerable/Dead reject는 그대로 상속.
	// 여기선 Asset Tag 하나만 더한다 — AS_Combat::PostGameplayEffectExecute가 GetAllAssetTags로 읽어 언블록 판정.
	// CDO 생성자에서 AddComponent<>() 직접 호출 금지(UE5.6 fatal) → CreateDefaultSubobject + GEComponents.Add.
	UAssetTagsGameplayEffectComponent* AssetTagsComp =
		CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(TEXT("UnblockableAssetTagComp"));

	FInheritedTagContainer TagChanges;
	TagChanges.Added.AddTag(GameplayTags::Ability_Combat_Unblockable);
	AssetTagsComp->SetAndApplyAssetTagChanges(TagChanges);

	GEComponents.Add(AssetTagsComp);
}
