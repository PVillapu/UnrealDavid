#include "GargoylePiece.h"

void AGargoylePiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);
}

void AGargoylePiece::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	Super::OnPieceDestroyed(PieceInstigator);

	// Update game card data
	GameCardData.PieceAttack += 2;
	GameCardData.PieceHealth += 2;

	LogPieceEvent(FString::Printf(TEXT("Stats incremented | Attack: %d, Health: %d"), GameCardData.PieceAttack, GameCardData.PieceHealth));
}
