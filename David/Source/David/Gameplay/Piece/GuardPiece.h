#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "GuardPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix

This piece can attack at his 3 front squares simultaneously

*/
UCLASS()
class DAVID_API AGuardPiece : public APieceActor
{
	GENERATED_BODY()

public:	
	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

private:
	bool SearchAndAttackEnemies(int32 FrontSquare);
};
