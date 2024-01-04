#include "PieceActor.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "../Cards/CardData.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void APieceActor::SetupPiece(ABoardManager* BoardManagerActor, const FGameCardData& GameCardData, const FCardData& CardData)
{
	BoardManager = BoardManager;

	SkeletalMeshComponent->SetSkeletalMesh(CardData.PieceMesh);
	CurrentHealth = BaseHealth = GameCardData.PieceHealth;
	CurrentAttack = BaseAttack = GameCardData.PieceAttack;
}

void APieceActor::ProcessTurn()
{
}

void APieceActor::DeployInSquare(int32 SquareIndex)
{
	OnDeployPieceInSquare(SquareIndex);
	Client_DeployPieceInSquare(SquareIndex);
}

void APieceActor::OnDeployPieceInSquare(int32 SquareIndex)
{
	FVector DeployLocation = BoardManager->GetSquareLocation(SquareIndex);
	SetActorLocation(DeployLocation);
}

void APieceActor::RegisterPieceAction(const FPieceAction& PieceAction) const
{
}

void APieceActor::Client_DeployPieceInSquare_Implementation(int32 SquareIndex)
{
	OnDeployPieceInSquare(SquareIndex);
}
