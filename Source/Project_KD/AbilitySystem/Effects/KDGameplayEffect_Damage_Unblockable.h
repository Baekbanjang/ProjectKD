#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/KDGameplayEffect_Damage_Physical.h"
#include "KDGameplayEffect_Damage_Unblockable.generated.h"

// 노랑(언블록) 공격용 데미지 GE. GE_Damage_Physical과 동일한 데미지/리젝 로직을 상속하고,
// Asset Tags에 Ability.Combat.Unblockable만 추가한다 → AS_Combat이 GetAllAssetTags로 읽어 패링 분기를
// 전부 스킵(정면이어도 못 막음, 회피 강제). 도끼 엘리트 슬램·활 차지샷이 DamageEffectClass로 지정.
UCLASS()
class PROJECT_KD_API UKDGameplayEffect_Damage_Unblockable : public UKDGameplayEffect_Damage_Physical
{
	GENERATED_BODY()

public:
	UKDGameplayEffect_Damage_Unblockable();
};
