#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Cards/GameCardData.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardSquare.h"
#include "../Player/DavidPlayerController.h"
#include "../Cards/GameCardData.h"
#include "../DavidGameState.h"
#include "../Misc/CustomDavidLogs.h"

ABoardManager::ABoardManager()
{
	// Only server needs to tick this actor
	if(HasAuthority()) PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	bProcessingAction = false;
}

void ABoardManager::Tick(float DeltaSeconds)
{
	if (!ActionsQueue.IsEmpty() && !bProcessingAction) 
	{
		PlayNextGameAction();
	}
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

void ABoardManager::RegisterGameAction(const FTurnAction& TurnAction)
{
	NetMulticast_SendGameAction(TurnAction);
}

FVector ABoardManager::GetSquareLocation(int32 SquareIndex)
{
	if(SquareIndex < 0 || SquareIndex >= BoardHeight * BoardWidth) return FVector();

	return BoardSquares[SquareIndex]->GetTransform().GetLocation(); 
}

void ABoardManager::CalculatePlayersScore(int32& Player1Score, int32& Player2Score)
{
	Player1Score = Player2Score = 0;

	for (ABoardSquare* Square : BoardSquares)
	{
		EDavidSquareColor Color = Square->GetSquareColor();
		if (Color == EDavidSquareColor::NEUTRAL) continue;
		else if (Color == EDavidSquareColor::PLAYER_1_COLOR) ++Player1Score;
		else ++Player2Score;
	}
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

void ABoardManager::ProcessPlayerEndTurn(EDavidPlayer PlayerTurn)
{
	// Clear the queue to start storing turn actions
	APieceActor* PieceToProcess = nullptr;

	// Call OnBeginTurn of each piece to be processed
	for (const TPair<int32, APieceActor*>& Pair : ServerBoardPieces)
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

	// All Actions played event
	FTurnAction AllActionsPlayed;
	AllActionsPlayed.ActionType = EDavidGameAction::END_TURN_ACTIONS_PLAYED;
	RegisterGameAction(AllActionsPlayed);
}

void ABoardManager::NetMulticast_SendGameAction_Implementation(FTurnAction TurnAction)
{
	ActionsQueue.Add(TurnAction);
}

void ABoardManager::PlayCardInSquare(FGameCardData& GameCardData, int32 SquareID, EDavidPlayer Player)
{
	// Create piece ID
	int32 PieceID = PieceIdCounter++;

	FTurnAction GameAction;
	GameAction.ActionType = EDavidGameAction::PLAY_CARD;
	GameAction.Payload.SetNum(sizeof(FGameCardData) + 2 * sizeof(int32) + sizeof(EDavidPlayer));
	FMemory::Memcpy(GameAction.Payload.GetData(), &PieceID, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[4], &SquareID, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[8], &Player, sizeof(EDavidPlayer));
	FMemory::Memcpy(&GameAction.Payload[8 + sizeof(EDavidPlayer)], &GameCardData, sizeof(FGameCardData));

	RegisterGameAction(GameAction);

	InstantiateAndRegisterPiece(GameCardData, SquareID, PieceID, Player);
}

void ABoardManager::PlayPieceAction(const FTurnAction& TurnAction)
{
	FPieceAction PieceAction = APieceActor::GetPieceAction(TurnAction);

	// Find the piece that needs to process the action
	if (ActiveBoardPieces.Contains(PieceAction.PieceID)) 
	{
		APieceActor* PieceActor = *ActiveBoardPieces.Find(PieceAction.PieceID);
		if (PieceActor)
		{
			// Process the action
			PieceActor->ProcessAction(PieceAction);
		}
	}
}

void ABoardManager::PlayCardInSquareAction(const FTurnAction& GameAction)
{
	int32 PieceID;
	int32 SquareID;
	EDavidPlayer Player;
	FGameCardData GameCardData;

	FMemory::Memcpy(&PieceID, GameAction.Payload.GetData(), sizeof(int32));
	FMemory::Memcpy(&SquareID, &GameAction.Payload[4], sizeof(int32));
	FMemory::Memcpy(&Player, &GameAction.Payload[8], sizeof(EDavidPlayer));

	APieceActor* PieceInstance;

	// Instantiate if im the client
	if (!HasAuthority())
	{
		FMemory::Memcpy(&GameCardData, &GameAction.Payload[8 + sizeof(EDavidPlayer)], sizeof(FGameCardData));

		PieceInstance = InstantiateAndRegisterPiece(GameCardData, SquareID, PieceID, Player);
	}
	else
	{
		// Get piece reference
		PieceInstance = *ActiveBoardPieces.Find(PieceID);
	}

	// Deploy the new piece in the square
	if (PieceInstance) 
	{
		PieceInstance->OnDeployPieceInSquareAction(SquareID);
		BoardSquares[SquareID]->Action_SetSquarePlayerColor(Player);
	}
}

APieceActor* ABoardManager::InstantiateAndRegisterPiece(const FGameCardData& GameCardData, const int32 SquareID, const int32 PieceID, const EDavidPlayer Player)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());
	if (PlayerController == nullptr) return nullptr;

	// Get Card data
	UDataTable* CardsDataTable = PlayerController->GetCardsDataTable();
	FCardData* CardData = CardsDataTable->FindRow<FCardData>(GameCardData.CardName, "");

	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData->CardPieceActor);

	// Register piece
	ActiveBoardPieces.Add(PieceID, PieceInstance);

	// Setup piece
	PieceInstance->SetupPiece(this, GameCardData, *CardData, PieceID, Player);

	// Update board square reference
	BoardSquares[SquareID]->SetPieceInSquare(PieceInstance);
	PieceInstance->SetBoardSquare(BoardSquares[SquareID]);

	// Update board state in server
	if (HasAuthority())
	{
		ServerBoardPieces.Add(PieceID, PieceInstance);

		BoardSquares[SquareID]->Process_SetSquarePlayerColor(Player);
	}

	return PieceInstance;
}

void ABoardManager::PlayNextGameAction()
{
	if (ActionsQueue.IsEmpty())	return;
	bProcessingAction = true;
	
	const FTurnAction TurnAction = ActionsQueue[0];
	ActionsQueue.RemoveAt(0);

	switch (TurnAction.ActionType)
	{
		case EDavidGameAction::PLAY_CARD:
		{
			PlayCardInSquareAction(TurnAction);
			break;
		}
		case EDavidGameAction::PIECE_ACTION:
		{
			PlayPieceAction(TurnAction);
			break;
		}
		case EDavidGameAction::END_TURN_ACTIONS_PLAYED:
		{
			ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(GetWorld()->GetFirstPlayerController());

			if (PlayerController) PlayerController->OnTurnActionsCompleted();

			OnGameActionComplete();

			break;
		}
		default: break;
	}
}

void ABoardManager::OnGameActionComplete()
{
	bProcessingAction = false;
}

void ABoardManager::OnPieceDeath(APieceActor* Piece)
{
	if (Piece == nullptr) return;

	const ABoardSquare* BoardSquare = Piece->GetBoardSquare();
	const int32 PieceLocation = BoardSquare->GetSquareIndex();

	// Remove the piece from board
	Piece->SetBoardSquare(nullptr);
	ServerBoardPieces.Remove(Piece->GetPieceID());
	BoardSquares[PieceLocation]->SetPieceInSquare(nullptr);
}

void ABoardManager::RemoveActivePiece(APieceActor* Piece)
{
	ActiveBoardPieces.Remove(Piece->GetPieceID());
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

ABoardSquare* ABoardManager::GetBoardSquare(int32 BoardIndex) const
{
	if (BoardIndex < 0 || BoardIndex >= BoardHeight * BoardWidth) return nullptr;

	return BoardSquares[BoardIndex];
}

void ABoardManager::GenerateBoardSquares()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Set the spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	BoardSquares.Empty();

	FVector SquaresGlobalOffset = GetActorLocation();

	for (int i = 0; i < BoardWidth * BoardHeight; ++i)
	{
		const int row = i / BoardHeight;
		const int col = i % BoardHeight;

		// Set the spawn location and rotation
		const FVector SpawnLocation = FVector(col * SquaresOffset, + row * SquaresOffset, 0.0f) + SquaresGlobalOffset;
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
