#include "PieceActor.h"
#include "../Board/BoardManager.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
}

void APieceActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APieceActor::SetupPiece(ABoardManager* BoardManager)
{
	Board = BoardManager;
}

void APieceActor::ProcessTurn()
{
}

void APieceActor::ProcessAction(int32 ActionID)
{
}
