#include "GargoylePiece.h"

void AGargoylePiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);

	if(!StatsMap.Contains(DeathCountKey))
		StatsMap.Add(TTuple<FName, int32>(DeathCountKey, 0));
}

void AGargoylePiece::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	Super::OnPieceDestroyed(PieceInstigator);

	if (StatsMap.Contains(DeathCountKey))
	{
		int32* DeathCount = StatsMap.Find(DeathCountKey);
		*DeathCount++;
	}

	// Update game card data...
}
