#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Cards/GameCardData.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardSquare.h"
#include "../Player/DavidPlayerController.h"
#include "../Cards/GameCardData.h"
#include "../DavidGameState.h"

ABoardManager::ABoardManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABoardManager::InitializeBoard()
{
	GenerateBoardSquares();
	PieceIdCounter = 0;

	if (UWorld* World = GetWorld())
	{
		ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());

		if (PlayerController)
			PlayerController->InitializationPartDone(EDavidPreMatchInitialization::BOARD_INITIALIZED);
	}
}

void ABoardManager::RegisterTurnAction(const FPieceAction& PieceAction)
{
	TurnActionsQueue.Add(PieceAction);
}

void ABoardManager::PlayCardInSquare(FGameCardData& GameCardData, int32 SquareID, EDavidPlayer Player)
{
	// Create piece ID
	int32 PieceID = PieceIdCounter++;

	// Deploy piece in board
	NetMulticast_DeployPieceInSquare(GameCardData, SquareID, PieceID, Player);
}

void ABoardManager::NetMulticast_DeployPieceInSquare_Implementation(FGameCardData GameCardData, int32 SquareID, int32 PieceID, EDavidPlayer Player)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());
	if (PlayerController == nullptr) return;

	// Get Card data
	UDataTable* CardsDataTable = PlayerController->GetCardsDataTable();
	FCardData* CardData = CardsDataTable->FindRow<FCardData>(GameCardData.CardName, "");

	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData->CardPieceActor);

	// Register piece
	BoardPieces.Add(PieceID, PieceInstance);

	// Setup piece
	PieceInstance->SetupPiece(this, GameCardData, *CardData, PieceID, Player);

	// Deploy the new piece in the square
	PieceInstance->DeployInSquare(SquareID);

	// Update board state in server
	if (HasAuthority())
	{
		BoardSquares[SquareID]->SetPieceInSquare(PieceInstance);
		PieceInstance->SetBoardSquare(BoardSquares[SquareID]);
	}
}

FVector ABoardManager::GetSquareLocation(int32 SquareIndex)
{
	if(SquareIndex < 0 || SquareIndex >= BoardHeight * BoardWidth) return FVector();

	return BoardSquares[SquareIndex]->GetTransform().GetLocation(); 
}

bool ABoardManager::CanPlayerPlayCardInSquare(EDavidPlayer Player, int32 SquareID)
{
	// Check for invalid SquareID
	if (SquareID < 0 || SquareID >= BoardHeight * BoardWidth) return false;

	// Check if there is a piece in the desired play square
	if (BoardSquares[SquareID]->GetPieceInSquare() != nullptr) return false;

	// Check if the square is in the deployment line of the player
	return (Player == EDavidPlayer::PLAYER_1 && SquareID < BoardWidth) ||
		(Player == EDavidPlayer::PLAYER_2 && SquareID >= BoardHeight * (BoardWidth - 1));
}

APieceActor* FindNextPieceToProcess(const TArray<ABoardSquare*>& BoardSquares, EDavidPlayer Player)
{
	// Search for the next piece of the board to be processed in this turn
	for (int i = 0; i < BoardSquares.Num(); ++i)
	{
		int32 SquareIndex = Player == EDavidPlayer::PLAYER_1 ? i : (BoardSquares.Num() - 1) - i;

		APieceActor* Piece = BoardSquares[SquareIndex]->GetPieceInSquare();
		if (Piece == nullptr || Piece->GetOwnerPlayer() != Player || Piece->HasBeenProcessed()) continue;

		// Found the next piece to be processed
		return Piece;
	}

	return nullptr;
}

void ABoardManager::ProcessPlayerTurn(EDavidPlayer PlayerTurn)
{
	// Clear the queue to start storing turn actions
	TurnActionsQueue.Empty();
	APieceActor* PieceToProcess = nullptr;

	// Call OnBeginTurn of each piece to be processed
	for (const TPair<int32, APieceActor*>& Pair : BoardPieces)
	{
		if (Pair.Value->GetOwnerPlayer() == PlayerTurn)
			Pair.Value->OnBeginTurn();
	}

	// Process each player turn
	do
	{
		PieceToProcess = FindNextPieceToProcess(BoardSquares, PlayerTurn);
		if (PieceToProcess) 
		{
			PieceToProcess->ProcessTurn();
		}
	} while (PieceToProcess);
}

void ABoardManager::SendTurnActions()
{
	NetMulticast_SendTurnActions(TurnActionsQueue);
}

void ABoardManager::NetMulticast_SendTurnActions_Implementation(const TArray<FPieceAction>& Actions)
{
	TurnActionsQueue = Actions;

	PlayTurnAction();
}

void ABoardManager::PlayTurnAction()
{
	// Once all actions has been processed, send a message to server and wait
	if (TurnActionsQueue.IsEmpty()) 
	{
		ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(GetWorld()->GetFirstPlayerController());
		
		if(PlayerController)
			PlayerController->Server_PlayerActionsProcessed();

		return;
	}

	// Pop the next piece action to process
	const FPieceAction PieceAction = TurnActionsQueue[0];
	TurnActionsQueue.RemoveAt(0);

	// Find the piece that needs to process the action
	APieceActor* PieceActor = *BoardPieces.Find(PieceAction.PieceID);
	if (PieceActor) 
	{
		// Process the action
		PieceActor->ProcessAction(PieceAction);
	}
}

void ABoardManager::OnActionComplete()
{
	PlayTurnAction();
}

bool ABoardManager::IsSquareOccupied(int32 Square) const
{
	return IsValidSquare(Square) && BoardSquares[Square]->GetPieceInSquare() != nullptr;
}

void ABoardManager::MovePieceToSquare(APieceActor* Piece, int32 TargetSquare)
{
	// Remove reference of the last board square
	if(Piece->GetBoardSquare())
		Piece->GetBoardSquare()->SetPieceInSquare(nullptr);

	// Update board and piece
	BoardSquares[TargetSquare]->SetPieceInSquare(Piece);
	Piece->SetBoardSquare(BoardSquares[TargetSquare]);
}

APieceActor* ABoardManager::GetPieceInSquare(int32 BoardSquare) const
{
	return BoardSquares[BoardSquare]->GetPieceInSquare();
}

void ABoardManager::GenerateBoardSquares()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Set the spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	BoardSquares.Empty();

	for (int i = 0; i < BoardWidth * BoardHeight; ++i)
	{
		const int row = i / BoardHeight;
		const int col = i % BoardHeight;

		// Set the spawn location and rotation
		const FVector SpawnLocation = FVector(col * SquaresOffset, row * SquaresOffset, 0.0f);
		const FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

		// Spawn the actor
		ABoardSquare* SpawnedSquare = World->SpawnActor<ABoardSquare>(BoardSquareBP, SpawnLocation, SpawnRotation, SpawnParams);
		SpawnedSquare->SetBoardManager(this);
		SpawnedSquare->SetSquareIndex(GetBoardIndex(row, col));

		// Add to BoardSquares
		if (SpawnedSquare)
		{
			BoardSquares.Add(SpawnedSquare);
		}
	}
}
