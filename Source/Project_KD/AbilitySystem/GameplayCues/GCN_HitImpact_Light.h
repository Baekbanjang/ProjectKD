#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_HitImpact_Light.generated.h"

// 피격 임팩트 큐. Event.Combat.Hit에서 발신(데미지 GE에서 분리).
// BP child: GameplayCueTag="GameplayCue.Combat.HitImpact.Light", OnExecute override로 SFX/VFX 재생.
UCLASS(Blueprintable)
class PROJECT_KD_API UGCN_HitImpact_Light : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGCN_HitImpact_Light();
};
