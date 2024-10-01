#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Cards/GameCardData.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardSquare.h"
#include "../Player/DavidPlayerController.h"
#include "../Cards/GameCardData.h"
#include "../DavidGameState.h"
#include "../Misc/CustomDavidLogs.h"
#include "../Player/PlayerCards.h"
#include "Kismet/GameplayStatics.h"
#include "SquareAction.h"
#include "../DavidPlayerState.h"
#include "../DavidGameInstance.h"

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
	PieceIdCounter = 0;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Get all BoardSquares from the world
	TArray<AActor*> LevelBoardSquares;
	UGameplayStatics::GetAllActorsOfClass(World, ABoardSquare::StaticClass(), LevelBoardSquares);

	// Add all BoardSquares to array
	for(AActor* Square : LevelBoardSquares)
	{
		ABoardSquare* BoardSquare = Cast<ABoardSquare>(Square);
		if (BoardSquare) 
		{
			BoardSquare->SetBoardManager(this);
			BoardSquares.Add(BoardSquare);
		}
	}

	// Sort the array by Square ID
	BoardSquares.Sort(
		[](const ABoardSquare& BoardSquare1, const ABoardSquare& BoardSquare2)
		{ 
			return BoardSquare1.GetSquareIndex() < BoardSquare2.GetSquareIndex(); 
		}
	);

	// Notice player controller of completed initialization
	ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());
	if (PlayerController)
		PlayerController->InitializationPartDone(EDavidPreMatchInitialization::BOARD_INITIALIZED);
}

void ABoardManager::RegisterGameAction(const FTurnAction& TurnAction)
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC && !PC->HasAuthority())
		{
			UE_LOG(LogTemp, Error, TEXT("Trying to register an action without being the server! *This should never happen*"));
			return;
		}
	}
	else return;

	NetMulticast_SendGameAction(TurnAction);
}

FVector ABoardManager::GetSquareLocation(int32 SquareIndex)
{
	if(SquareIndex < 0 || SquareIndex >= BoardSquares.Num()) return FVector();

	return BoardSquares[SquareIndex]->GetTransform().GetLocation(); 
}

FVector ABoardManager::GetSquarePieceLocation(int32 SquareIndex)
{
	if (SquareIndex < 0 || SquareIndex >= BoardSquares.Num()) return FVector();

	return BoardSquares[SquareIndex]->GetSquarePieceLocation();
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
	const FPieceAction PieceAction = APieceActor::GetPieceAction(TurnAction);

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

void ABoardManager::PlaySquareAction(const FTurnAction TurnAction)
{
	const FSquareAction SquareAction = ABoardSquare::GetSquareAction(TurnAction);

	if (SquareAction.SquareIndex >= 0 && SquareAction.SquareIndex < BoardSquares.Num())
	{
		BoardSquares[SquareAction.SquareIndex]->ProcessAction(SquareAction);
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
	}
	else 
	{
		OnGameActionComplete();
	}
}

APieceActor* ABoardManager::InstantiateAndRegisterPiece(FGameCardData& GameCardData, const int32 SquareID, const int32 PieceID, const EDavidPlayer Player)
{
	UDavidGameInstance* GameInstance = Cast<UDavidGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if(GameInstance == nullptr) return nullptr;
	
	// Get Card data
	TArray<FCardData>& CardsArray = GameInstance->GetGameCards();
	
	// Check for valid index
	if (GameCardData.CardDTIndex < 0 || GameCardData.CardDTIndex > CardsArray.Num())
	{
		UE_LOG(LogDavid, Error, TEXT("Invalid card index received to instantiate: %d"), GameCardData.CardDTIndex);
		return nullptr;
	}

	FCardData& CardData = CardsArray[GameCardData.CardDTIndex];

	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData.CardPieceClass);	// Crash in client

	// Register piece
	ActiveBoardPieces.Add(PieceID, PieceInstance);

	// Update board square reference
	BoardSquares[SquareID]->SetPieceInSquare(PieceInstance);
	PieceInstance->SetBoardSquare(BoardSquares[SquareID]);

	// Setup piece
	PieceInstance->SetupPiece(this, GameCardData, PieceID, Player);

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
		case EDavidGameAction::SQUARE_ACTION:
		{
			PlaySquareAction(TurnAction);
			break;
		}
		default: break;
	}
}

void ABoardManager::OnGameActionComplete()
{
	bProcessingAction = false;
}

void ABoardManager::OnPieceDeath(APieceActor* Piece, APieceActor* InstigatorPiece)
{
	if (Piece == nullptr || InstigatorPiece == nullptr) return;

	Piece->OnPieceDestroyed(InstigatorPiece);

	OnPieceDestroyed.Broadcast(Piece, InstigatorPiece);

	if (ADavidGameState* GameState = GetWorld()->GetGameState<ADavidGameState>())
	{
		if (ADavidPlayerController* PlayerController = GameState->GetPlayerController(Piece->GetOwnerPlayer()))
		{			
			PlayerController->GetPlayerCards()->PutCardOnDeck(Piece->GetDeathCard());
		}
	}
}

void ABoardManager::Process_RemovePieceFromProcessBoard(APieceActor* PieceToRemove)
{
	UE_LOG(LogDavidGameEvent, Log, TEXT("Process_RemovePieceFromProcessBoard | Target: %d"), PieceToRemove->GetBoardSquare()->GetSquareIndex());
	const ABoardSquare* BoardSquare = PieceToRemove->GetBoardSquare();
	const int32 PieceLocation = BoardSquare->GetSquareIndex();

	BoardSquares[PieceLocation]->SetPieceInSquare(nullptr);
	ServerBoardPieces.Remove(PieceToRemove->GetPieceID());
}

void ABoardManager::Process_AttackPiece(APieceActor* PieceToAttack, APieceActor* DamageCauser, int32 AttackAmmount)
{
	UE_LOG(LogDavidGameEvent, Log, TEXT("Process_AttackPiece | Target: %d"), PieceToAttack->GetBoardSquare()->GetSquareIndex());
	PieceToAttack->Process_TakeDamage(AttackAmmount);
	PieceToAttack->Process_HoldDamage(DamageCauser);
}

void ABoardManager::Process_AttackMultiplePieces(TArray<APieceActor *> &PiecesToAttack, APieceActor *DamageCauser, int32 AttackAmmount)
{
	UE_LOG(LogDavidGameEvent, Log, TEXT("Process_AttackMultiplePieces | TargetCount: %d"), PiecesToAttack.Num());

	// First deal the damage to each piece
	for(APieceActor* Piece : PiecesToAttack)
	{
		if(Piece)
		{
			UE_LOG(LogDavidGameEvent, Log, TEXT("Process_AttackMultiplePieces | Target: %d"), Piece->GetBoardSquare()->GetSquareIndex());
			Piece->Process_TakeDamage(AttackAmmount);
		}
	}

	// Once the damage is been aplied and board updates, each piece reacts to it
	for(APieceActor* Piece : PiecesToAttack)
	{
		if(Piece)
		{
			Piece->Process_HoldDamage(DamageCauser);
		}
	}
}

void ABoardManager::RemoveActivePiece(APieceActor* Piece)
{
	ActiveBoardPieces.Remove(Piece->GetPieceID());
	Piece->Destroy();
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

	// Paint the square
	if (ABoardSquare* BoardSquare = GetBoardSquare(TargetSquare))
		BoardSquare->Process_SetSquarePlayerColor(Piece->GetOwnerPlayer());

	// Check endline piece
	CheckIfAnyPieceFinished();
}

void ABoardManager::CheckIfAnyPieceFinished()
{
	// Iterate over a copy map to avoid delete during the iteration
	TMap<int32, APieceActor*> BoardPiecesCopy = TMap<int32, APieceActor*>(ServerBoardPieces);

	// Check for any endline piece
	for (const TPair<int32, APieceActor*>& pair : BoardPiecesCopy)
	{
		APieceActor* Piece = pair.Value;

		if (!Piece) continue;

		const int32 SquareIndex = Piece->GetBoardSquare()->GetSquareIndex();

		if ((Piece->GetOwnerPlayer() == EDavidPlayer::PLAYER_1 && SquareIndex >= (BoardHeight - 1) * BoardWidth)	/* Player 1 */
			|| (Piece->GetOwnerPlayer() == EDavidPlayer::PLAYER_2 && SquareIndex < BoardWidth))						/* Player 2 */
		{
			Piece->OnPieceReachedEndLine();
			OnPieceReachedEndline(Piece);
		}
	}
}

void ABoardManager::OnPieceReachedEndline(APieceActor* Piece)
{
	// Get column and operations data
	const EDavidPlayer ScorePlayer = Piece->GetOwnerPlayer();
	const int32 SquareIndex = Piece->GetBoardSquare()->GetSquareIndex();
	const int32 MultiplyFactor = ScorePlayer == EDavidPlayer::PLAYER_1 ? -1 : 1;

	// Paint and lock all squares in the column
	for (int i = 0; i < BoardHeight; ++i)
	{
		int32 SquareToPaint = SquareIndex + BoardWidth * i * MultiplyFactor;
		BoardSquares[SquareToPaint]->Process_SetSquarePlayerColor(ScorePlayer);
		BoardSquares[SquareToPaint]->Process_LockSquare();
	}
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