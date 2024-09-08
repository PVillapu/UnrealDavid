#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "CatapultPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix
*/
UCLASS()
class DAVID_API ACatapultPiece : public APieceActor
{
	GENERATED_BODY()

public:	

	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn() override;

private:
	bool SearchAndAttackArea(int32 TargetMiddleSquare);

private:

	UPROPERTY(EditAnywhere, Category="Piece Values")
	int32 AttackRange = 2;
};
