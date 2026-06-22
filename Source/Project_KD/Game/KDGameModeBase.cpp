// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/KDGameModeBase.h"
#include "Player/KDPlayerCharacter.h"
#include "Player/KDPlayerController.h"
#include "Player/KDPlayerState.h"

AKDGameModeBase::AKDGameModeBase()
{
	// C++ defaults; the BP child overrides DefaultPawnClass with the BP pawn (mesh/anim).
	DefaultPawnClass = AKDPlayerCharacter::StaticClass();
	PlayerControllerClass = AKDPlayerController::StaticClass();
	PlayerStateClass = AKDPlayerState::StaticClass();
}
