#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "KDInputBufferComponent.generated.h"

USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	FGameplayTag Tag;
	float TimestampSec = 0.f;
};

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class PROJECT_KD_API UKDInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKDInputBufferComponent();

	// 입력 보관 기한 — 상한 0.8 = SB 입력 접수창 0.7~0.8
	UPROPERTY(EditAnywhere, Category="Input Buffer", meta=(ClampMin="0.05", ClampMax="0.8"))
	float BufferTimeWindow = 0.2f;

	UPROPERTY(EditAnywhere, Category="Input Buffer", meta=(ClampMin="1", ClampMax="10"))
	int32 MaxBufferSize = 4;

	UFUNCTION(BlueprintCallable, Category="Input Buffer")
	void Push(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable, Category="Input Buffer")
	bool TryConsume(FGameplayTag DesiredTag);

	UFUNCTION(BlueprintCallable, Category="Input Buffer")
	void Clear();

private:
	TArray<FBufferedInput> Buffer;

	void PruneExpired();
};
