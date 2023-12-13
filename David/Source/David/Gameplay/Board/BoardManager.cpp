#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Piece/PieceActor.h"

ABoardManager::ABoardManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABoardManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));

	InitializeBoard();
	GenerateBoardSquares();
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
		AActor* const SpawnedActor = World->SpawnActor<AActor>(BoardSquareBP, SpawnLocation, SpawnRotation, SpawnParams);

		// Add to BoardSquares
		if (SpawnedActor)
		{
			BoardSquares.Add(SpawnedActor);
		}
	}
}

void ABoardManager::PlayCardInSquare(const FCardData& CardData, int32 Column, int32 Row)
{
	// We asume that the coords are valid and piece can be played at coords
	int32 PieceId = PieceIdCounter++;
	Board[GetBoardIndex(Row, Column)] = PieceId;

	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData.CardPieceActor);
	PieceInstance->SetupPiece(this, PieceId);

	// Place the new piece in the square
	FVector TargetSquareLocation = BoardSquares[(Row * BoardHeight) + Column]->GetActorLocation();
	PieceInstance->SetActorLocation(TargetSquareLocation);
}

void ABoardManager::InitializeBoard()
{
	// Setup board
	Board.Init(-1, BoardHeight * BoardWidth);

	PieceIdCounter = 0;
}
