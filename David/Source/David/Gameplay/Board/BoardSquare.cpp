#include "BoardSquare.h"

ABoardSquare::ABoardSquare()
{
	PrimaryActorTick.bCanEverTick = false;
	BoardManager = nullptr;
	PieceInSquare = nullptr;
	SquareIndex = -1;

	SquareMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Square mesh"));
	SquareMesh->SetupAttachment(RootComponent);
}

