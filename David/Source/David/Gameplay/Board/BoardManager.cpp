#include "BoardManager.h"

ABoardManager::ABoardManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABoardManager::GenerateBoardSquares()
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;
	
    // Set the spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

	for (int i = 0; i < BoardWidth * BoardHeight; ++i) 
	{
		int row = i / BoardHeight;
		int col = i % BoardHeight;

		// Set the spawn location and rotation
		FVector SpawnLocation = FVector(row * SquaresOffset, col * SquaresOffset, 0.0f);
		FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

		// Spawn the actor
		AActor* const SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
	}
}

void ABoardManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateBoardSquares();
}

