#include "BoardManager.h"

ABoardManager::ABoardManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABoardManager::BeginPlay()
{
	Super::BeginPlay();

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
		int row = i / BoardHeight;
		int col = i % BoardHeight;

		// Set the spawn location and rotation
		FVector SpawnLocation = FVector(row * SquaresOffset, col * SquaresOffset, 0.0f);
		FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

		// Spawn the actor
		AActor* const SpawnedActor = World->SpawnActor<AActor>(BoardSquareBP, SpawnLocation, SpawnRotation, SpawnParams);

		// Add to BoardSquares
		if (SpawnedActor)
		{
			BoardSquares.Add(SpawnedActor);
		}
	}
}

void ABoardManager::InitializeBoard()
{
	// Setup board
	Board.SetNum(BoardHeight);

	// Init the array with all positions with -1 value
	for (int i = 0; i < BoardHeight; ++i)
	{
		Board[i].Init(-1, BoardWidth);
	}
}
