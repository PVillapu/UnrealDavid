#include "BoardManager.h"
#include "../Cards/CardData.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardSquare.h"

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
}


void ABoardManager::InitializeBoard()
{
	GenerateBoardSquares();
	PieceIdCounter = 0;
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

void ABoardManager::PlayCardInSquare(const FCardData& CardData, int32 Square, int32 PieceID)
{
	// Spawn the piece actor
	APieceActor* PieceInstance = GetWorld()->SpawnActor<APieceActor>(CardData.CardPieceActor);
	PieceInstance->SetupPiece(this);

	// Place the new piece in the square
	FVector TargetSquareLocation = BoardSquares[Square]->GetActorLocation();
	PieceInstance->SetActorLocation(TargetSquareLocation);

	// Register piece
	BoardPieces.Add(TTuple<int32, APieceActor*>(PieceID, PieceInstance));
}

void ABoardManager::Server_PlayCardRequest_Implementation(const FString& CardName, int32 BoardIndex)
{
}

void ABoardManager::PlayCard_Implementation(const FString& CardName, int32 BoardIndex)
{
}