#include "Input/InputBufferComponent.h"

#include "Engine/World.h"

UInputBufferComponent::UInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInputBufferComponent::Push(FGameplayTag InputTag)
{
	if (!InputTag.IsValid()) return;

	PruneExpired();

	const UWorld* World = GetWorld();
	if (!World) return;

	FBufferedInput NewEntry;
	NewEntry.Tag = InputTag;
	NewEntry.TimestampSec = World->GetTimeSeconds();
	Buffer.Add(NewEntry);

	while (Buffer.Num() > MaxBufferSize)
	{
		Buffer.RemoveAt(0);
	}
}

bool UInputBufferComponent::TryConsume(FGameplayTag DesiredTag)
{
	if (!DesiredTag.IsValid()) return false;

	PruneExpired();

	for (int32 i = 0; i < Buffer.Num(); ++i)
	{
		if (Buffer[i].Tag.MatchesTagExact(DesiredTag))
		{
			Buffer.RemoveAt(i);
			return true;
		}
	}
	return false;
}

void UInputBufferComponent::Clear()
{
	Buffer.Reset();
}

void UInputBufferComponent::PruneExpired()
{
	const UWorld* World = GetWorld();
	if (!World) return;

	const float Now = World->GetTimeSeconds();
	Buffer.RemoveAll([this, Now](const FBufferedInput& Entry)
	{
		return (Now - Entry.TimestampSec) > BufferTimeWindow;
	});
}
