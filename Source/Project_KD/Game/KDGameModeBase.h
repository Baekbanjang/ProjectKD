// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KDGameModeBase.generated.h"

// Single-player action prototype game mode. Wires the project's C++ framework classes
// (Pawn / PlayerController / PlayerState). A BP child sets the BP pawn (mesh/anim) per map
// and is selected as the map's GameMode Override in the editor.
UCLASS()
class PROJECT_KD_API AKDGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKDGameModeBase();
};
