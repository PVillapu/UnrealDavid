#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "DiagonalAttackerPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix
*/
UCLASS()
class DAVID_API ADiagonalAttackerPiece : public APieceActor
{
	GENERATED_BODY()

public:	
	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

protected:

	bool Process_AttackPieceInDiagonals();

	bool SearchDiagonalTargets(int32 TargetSquare, bool LeftSide, TArray<int32>& SquaresToAttack);
};
