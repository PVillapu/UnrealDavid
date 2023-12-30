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