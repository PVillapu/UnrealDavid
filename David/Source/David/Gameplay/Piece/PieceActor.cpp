#include "PieceActor.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "../Cards/CardData.h"
#include "Kismet/GameplayStatics.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void APieceActor::SetupPiece(ABoardManager* BoardManagerActor, const FGameCardData& GameCardData, const FCardData& CardData, int32 ID, EDavidPlayer PieceOwner)
{
	BoardManager = BoardManager;

	CurrentHealth = BaseHealth = GameCardData.PieceHealth;
	CurrentAttack = BaseAttack = GameCardData.PieceAttack;

	PieceID = ID;
	PlayerOwner = PieceOwner;
}

void APieceActor::ProcessTurn()
{
	bHasBeenProcessed = true;
}

void APieceActor::ProcessAction(const FPieceAction& Action)
{
}

void APieceActor::OnBeginTurn()
{
	bHasBeenProcessed = false;
}

void APieceActor::RegisterPieceAction(EPieceAction PieceAction) const
{
	FPieceAction* Action = new FPieceAction();
	Action->ActionID = PieceAction;
	Action->PieceID = PieceID;

	BoardManager->AddTurnAction(Action);
}

void APieceActor::DeployInSquare(int32 SquareIndex)
{
	Multicast_DeployPieceInSquare(SquareIndex);
}

void APieceActor::OnDeployPieceInSquare(int32 SquareIndex)
{
	FVector DeployLocation = BoardManager->GetSquareLocation(SquareIndex);
	SetActorLocation(DeployLocation);
}

void APieceActor::Multicast_DeployPieceInSquare_Implementation(int32 SquareIndex)
{
	OnDeployPieceInSquare(SquareIndex);
}
