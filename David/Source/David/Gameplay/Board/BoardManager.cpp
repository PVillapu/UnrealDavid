#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Cards/GameCardData.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardSquare.h"
#include "../Player/DavidPlayerController.h"

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
		const FVector SpawnLocation = FVector(row * SquaresOffset, col * SquaresOffset, 0.0f);
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

void ABoardManager::PlayCardInSquare(const FCardData& CardData, const FGameCardData& GameCardData, int32 Square)
{
	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData.CardPieceActor);
	PieceInstance->SetupPiece(this, GameCardData, CardData);

	// Register piece
	int32 PieceID = PieceIdCounter++;
	BoardPieces.Add(PieceID, PieceInstance);

	// Deploy the new piece in the square
	PieceInstance->DeployInSquare(Square);
}

void ABoardManager::ProcessPlayerTurn(EDavidPlayer PlayerTurn)
{

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
