#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputBufferComponent.generated.h"

USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	FGameplayTag Tag;
	float TimestampSec = 0.f;
};

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class PROJECT_KD_API UInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInputBufferComponent();

	UPROPERTY(EditAnywhere, Category="Input Buffer", meta=(ClampMin="0.05", ClampMax="0.5"))
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
