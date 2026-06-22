// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Combo/ComboComponent.h"

#include "ComboTreeDataAsset.h"

UComboComponent::UComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const FComboBranch* UComboComponent::ProcessInput(FGameplayTag InputTag, EComboContext Context)
{
	if (Context != LastContext)
	{
		InputHistory.Reset();
		LastInputTime = -1.f;
		LastContext = Context;
	}
	
	if (const UWorld* World = GetWorld())
	{
		const float Now = World->GetTimeSeconds();

		// 콤보 1타 시
		if (LastInputTime < 0.f)
		{
			CurrentTempoMultiplier = 1.f; // 콤보 1타는 정상 속도
		}
		// 
		else
		{
			const float Interval = Now - LastInputTime; // 시간차 확인
			float Mult;
			if (TempoCurve)
			{
				Mult = TempoCurve->GetFloatValue(Interval); // 시간 차를 통해 Y값 추출
			}
			else
			{
				// 커브 없을 때: 0.25초 이하 -> 최대배속, 0.6초 이상 -> 1.0배, 사이는 보간.
				Mult = FMath::GetMappedRangeValueClamped(
					FVector2D(0.6f, 0.25f), FVector2D(1.f, MaxTempoMultiplier), Interval);
			}
			CurrentTempoMultiplier = FMath::Clamp(Mult, 1.f, MaxTempoMultiplier);
		}

		// 입력 시각 기록
		LastInputTime = Now;
	}
	
	InputHistory.Add(InputTag);

	// 리셋 Timer 재시작 (마지막 입력 후 ComboResetTime 내 다음 입력 없으면 자동 초기화).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ResetTimerHandle, this,
			&UComboComponent::OnResetTimeout, ComboResetTime, false);
	}

	UComboTreeDataAsset* Tree = (Context == EComboContext::Air) ? AirComboTree : ComboTree;
	if (!IsValid(Tree))
	{
		UE_LOG(LogTemp, Warning, TEXT("[KD] UComboComponent: ComboTree(ctx=%d) not assigned on %s"),
	    (int32)Context, *GetOwner()->GetName());
		return nullptr;
	}

	// 1단계: 정확 매칭 검사 — InputHistory 전체가 분기 InputSequence와 동일한가.
	for (const FComboBranch& Branch : Tree->Branches)
	{
		if (Branch.InputSequence.Num() != InputHistory.Num()) continue;

		bool bExactMatch = true;
		for (int32 i = 0; i < InputHistory.Num(); ++i)
		{
			if (Branch.InputSequence[i] != InputHistory[i])
			{
				bExactMatch = false;
				break;
			}
		}

		if (bExactMatch)
		{
			// 콤보 완료 — 분기 반환 + InputHistory 리셋 (다음 콤보는 새 시퀀스로 시작).
			const FComboBranch* MatchedBranch = &Branch;
			ClearHistory();
			return MatchedBranch;
		}
	}

	// 2단계: Prefix 매칭 검사 — InputHistory가 어느 분기의 시작 prefix인가.
	for (const FComboBranch& Branch : Tree->Branches)
	{
		if (Branch.InputSequence.Num() <= InputHistory.Num()) continue;

		bool bIsPrefix = true;
		for (int32 i = 0; i < InputHistory.Num(); ++i)
		{
			if (Branch.InputSequence[i] != InputHistory[i])
			{
				bIsPrefix = false;
				break;
			}
		}

		if (bIsPrefix)
		{
			// Prefix 진행 중 — GA는 부모 디폴트 재생 (AttackMontage 교체 안 함).
			return nullptr;
		}
	}

	// 3단계: 어느 prefix도 아님 — 마지막 입력만 남기고 새 시퀀스 시작.
	InputHistory.Reset();
	InputHistory.Add(InputTag);
	return nullptr;
}

void UComboComponent::ClearHistory()
{
	InputHistory.Reset();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetTimerHandle);
	}
}

void UComboComponent::OnResetTimeout()
{
	ClearHistory();
}

