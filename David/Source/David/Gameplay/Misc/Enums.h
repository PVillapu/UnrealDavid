#pragma once

#include "CoreMinimal.h"

// Enum to handle the different gameplay states
UENUM()
enum EDavidMatchState : int
{
	STARTING_GAME,
	END_GAME,
	PLAYER_1_TURN,
	PLAYER_2_TURN,
	PROCESSING_TURN
};

UENUM()
enum EDavidPlayer : int 
{
	PLAYER_1,
	PLAYER_2
};

UENUM()
enum EDavidPreMatchInitialization : int
{
	PLAYER_INITIALIZED = 0,
	BOARD_INITIALIZED,
	PLAYER_CARDS_INITIALIZED,
	PLAYER_HUD_INITIALIZED,
	MAX_VALUE	// Must be always the last value
};

UENUM()
enum EDavidSquareColor : int 
{
	PLAYER_1_COLOR = 0,
	PLAYER_2_COLOR,
	NEUTRAL
};