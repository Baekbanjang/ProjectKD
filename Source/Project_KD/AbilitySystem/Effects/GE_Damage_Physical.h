#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage_Physical.generated.h"

// Instant damage GE. IncomingDamage += SetByCaller(AttackPower) → AS_Combat이 Defense 경감 후 Health 차감.
// Invulnerable/Dead 태그 보유 대상은 자동 reject (TargetTagRequirementsComp).
UCLASS()
class PROJECT_KD_API UGE_Damage_Physical : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage_Physical();
};
