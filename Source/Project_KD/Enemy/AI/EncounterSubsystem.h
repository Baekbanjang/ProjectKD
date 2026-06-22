// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EncounterSubsystem.generated.h"

class AKDEnemyBaseCharacter;

// 그룹 교전 조율자. 동시 공격자를 MaxSimultaneousAttackers개로 제한(공격 "번호표").
// 책임: 토큰 grant/return(count cap). LOS·포지셔닝은 BTService 잔류. 적 간 직접 참조 0.
// 싱글 프로토 — authority-only, 복제 없음.
UCLASS()
class PROJECT_KD_API UEncounterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 공격 직전 BTService가 호출. 보유 중이면 true 유지 / 빈 슬롯 있으면 grant /
	// 꽉 차도 hold-time 초과 보유자 있으면 가장 오래된 1명 회수 후 grant(로테이션) / 아니면 false.
	bool RequestToken(AKDEnemyBaseCharacter* Enemy);

	// 경직·사망·range 이탈·소멸 시 호출. 슬롯 비워 대기 적이 받게 함. 멱등.
	void ReturnToken(AKDEnemyBaseCharacter* Enemy);

private:
	// 토큰 보유자 + grant 월드시각. count <= MaxSimultaneousAttackers 불변.
	struct FTokenSlot
	{
		TWeakObjectPtr<AKDEnemyBaseCharacter> Holder;
		float GrantTime = 0.f;
	};

	TArray<FTokenSlot> TokenHolders;

	// 동시 공격자 상한 / 강제 회수 시각은 CVar — kd.Encounter.MaxAttackers / kd.Encounter.TokenHoldTime. 멤버 아님.
};
