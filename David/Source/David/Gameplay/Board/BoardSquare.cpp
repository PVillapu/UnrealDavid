#include "BoardSquare.h"
#include "../DavidGameState.h"
#include "TurnAction.h"
#include "BoardManager.h"
#include "TimerManager.h"

ABoardSquare::ABoardSquare()
{
	PrimaryActorTick.bCanEverTick = false;
	BoardManager = nullptr;
	PieceInSquare = nullptr;
	SquareIndex = -1;

	SquareMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Square mesh"));
	SetRootComponent(SquareMesh);

	PieceBaseLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Piece base position"));
	PieceBaseLocation->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	PieceBaseLocation->SetupAttachment(RootComponent);

	SquareColor = ProcessSquareColor = EDavidSquareColor::NEUTRAL;
}

void ABoardSquare::ProcessAction(const FSquareAction& Action)
{
	switch (Action.ActionID) 
	{
		case EDavidSquareAction::PAINT_SQUARE:
		{
			Action_SetSquarePlayerColor(Action);
			break;
		}
		case EDavidSquareAction::LOCK_SQUARE:
		{
			Action_LockSquare();
			break;
		}
		default: break;
	}
}

void ABoardSquare::Process_SetSquarePlayerColor(EDavidPlayer Player)
{
	if (bIsLocked) return;

	ProcessSquareColor = Player == EDavidPlayer::PLAYER_1 ? EDavidSquareColor::PLAYER_1_COLOR : EDavidSquareColor::PLAYER_2_COLOR;

	TArray<uint8> Payload;
	Payload.SetNum(sizeof(EDavidPlayer));
	FMemory::Memcpy(Payload.GetData(), &Player, sizeof(EDavidPlayer));

	RegisterSquareAction(EDavidSquareAction::PAINT_SQUARE, Payload);
}

void ABoardSquare::Process_LockSquare()
{
	if (bIsLocked) return;

	bIsLocked = true;

	RegisterSquareAction(EDavidSquareAction::LOCK_SQUARE);
}

void ABoardSquare::Action_SetSquarePlayerColor(const FSquareAction& Action)
{
	// Get the player from the payload
	EDavidPlayer Player;
	FMemory::Memcpy(&Player, Action.Payload.GetData(), sizeof(EDavidPlayer));

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
	UMaterial* TargetMaterial = SquareColor == EDavidSquareColor::PLAYER_1_COLOR ? Player1SquareMaterial : Player2SquareMaterial;
	SquareMesh->SetMaterial(0, TargetMaterial);

	// WIP
	ABoardManager* BM = BoardManager;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [BM]() { BM->OnGameActionComplete(); }, 0.4f, false);
}

void ABoardSquare::Action_LockSquare()
{
	// TODO: Play some animations...
	UE_LOG(LogTemp, Display, TEXT("Locking Square %d"), SquareIndex);

	// WIP
	ABoardManager* BM = BoardManager;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [BM]() { BM->OnGameActionComplete(); }, 0.4f, false);
}

FSquareAction ABoardSquare::GetSquareAction(const FTurnAction& GameAction)
{
	int32 SquareIndex;
	int32 ActionID;
	TArray<uint8> Payload;
	Payload.SetNum(GameAction.Payload.Num() - 2 * sizeof(int32));

	FMemory::Memcpy(&SquareIndex, GameAction.Payload.GetData(), sizeof(int32));
	FMemory::Memcpy(&ActionID, &GameAction.Payload[4], sizeof(int32));

	// Retrieve payload only if there is one available
	if (Payload.Num() != 0)
		FMemory::Memcpy(Payload.GetData(), &GameAction.Payload[8], Payload.Num());

	return FSquareAction(SquareIndex, ActionID, Payload);
}

void ABoardSquare::RegisterSquareAction(int32 SquareAction)
{
	FTurnAction GameAction;
	GameAction.ActionType = EDavidGameAction::SQUARE_ACTION;
	GameAction.Payload.SetNum(2 * sizeof(int32));
	FMemory::Memcpy(GameAction.Payload.GetData(), &SquareIndex, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[4], &SquareAction, sizeof(int32));

	BoardManager->RegisterGameAction(GameAction);
}

void ABoardSquare::RegisterSquareAction(int32 SquareAction, const TArray<uint8>& Payload)
{
	FTurnAction GameAction;
	GameAction.ActionType = EDavidGameAction::SQUARE_ACTION;
	GameAction.Payload.SetNum(2 * sizeof(int32) + Payload.Num());
	FMemory::Memcpy(GameAction.Payload.GetData(), &SquareIndex, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[4], &SquareAction, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[8], Payload.GetData(), Payload.Num());

	BoardManager->RegisterGameAction(GameAction);
}
