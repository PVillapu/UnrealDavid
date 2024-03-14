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

protected:
	enum ESpearPieceAction : int32 { ThrowSpear = 4 };

public:	
	void SetupPiece(class ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner) override;

	/* Called by the server when the piece needs to be processed */
	void ProcessTurn() override;

	void ProcessAction(const FPieceAction& Action) override;

private:
	bool AttackEnemyInRange();

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 SpearDamage = 2;

private:

	bool bHasSpear = true;
};
