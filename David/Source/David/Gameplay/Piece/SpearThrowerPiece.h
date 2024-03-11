#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "SpearThrowerPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix

If there is an enemy piece in the same column where this piece is deployed, throws the spear and make X damage. If not, conserves the spear and 
attacks in a range of 2 squares.
*/
UCLASS()
class DAVID_API ASpearThrowerPiece : public APieceActor
{
	GENERATED_BODY()

public:	
	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn() override;

	virtual void ProcessAction(const FPieceAction& Action) override;

	/* Called when the piece has to be placed in the board */
	virtual void OnDeployPieceInSquareAction(int32 SquareIndex) override;
};
