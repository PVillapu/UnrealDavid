#include "BoardSquare.h"
#include "../DavidGameState.h"

ABoardSquare::ABoardSquare()
{
	PrimaryActorTick.bCanEverTick = false;
	BoardManager = nullptr;
	PieceInSquare = nullptr;
	SquareIndex = -1;

	SquareMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Square mesh"));
	SquareMesh->SetupAttachment(RootComponent);

	SquareColor = ProcessSquareColor = EDavidSquareColor::NEUTRAL;
}

void ABoardSquare::Process_SetSquarePlayerColor(EDavidPlayer Player)
{
	ProcessSquareColor = Player == EDavidPlayer::PLAYER_1 ? EDavidSquareColor::PLAYER_1_COLOR : EDavidSquareColor::PLAYER_2_COLOR;
}

void ABoardSquare::Action_SetSquarePlayerColor(EDavidPlayer Player)
{
	if (bIsLocked) return;

	// If the square was neutral, increase player score
	if (SquareColor == EDavidSquareColor::NEUTRAL)
	{
		if (UWorld* World = GetWorld())
		{
			ADavidGameState* GameState = World->GetGameState<ADavidGameState>();
			GameState->Action_IncreasePlayerScore(Player, 1);
		}
	}
	// If square color changes from one player to another, increase new player score and decrease the other player score
	else if ((SquareColor == EDavidSquareColor::PLAYER_1_COLOR && Player == EDavidPlayer::PLAYER_2)
		|| (SquareColor == EDavidSquareColor::PLAYER_2_COLOR && Player == EDavidPlayer::PLAYER_1))
	{
		if (UWorld* World = GetWorld())
		{
			ADavidGameState* GameState = World->GetGameState<ADavidGameState>();
			EDavidPlayer OtherPlayer = Player == EDavidPlayer::PLAYER_1 ? EDavidPlayer::PLAYER_2 : EDavidPlayer::PLAYER_1;
			GameState->Action_IncreasePlayerScore(Player, 1);
			GameState->Action_IncreasePlayerScore(OtherPlayer, -1);
		}
	}

	SquareColor = Player == EDavidPlayer::PLAYER_1 ? EDavidSquareColor::PLAYER_1_COLOR : EDavidSquareColor::PLAYER_2_COLOR;
	ChangeSquareColor();
}

void ABoardSquare::ChangeSquareColor()
{
	UMaterial* TargetMaterial = SquareColor == EDavidSquareColor::PLAYER_1_COLOR ? Player1SquareMaterial : Player2SquareMaterial;
	SquareMesh->SetMaterial(0, TargetMaterial);
}
