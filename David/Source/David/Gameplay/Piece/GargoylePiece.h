#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "GargoylePiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix

This piece gains +2 +2 for each time that has died in the game
*/
UCLASS()
class DAVID_API AGargoylePiece : public APieceActor
{
	GENERATED_BODY()

public:	
	void SetupPiece(class ABoardManager* BoardManagerActor, FGameCardData& GameCardData, int32 ID, EDavidPlayer PieceOwner) override;

	void OnPieceDestroyed(APieceActor* PieceInstigator) override;

private:

	const FName DeathCountKey = "DeathCount";
};
