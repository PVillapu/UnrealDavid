#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceAction.h"
#include "../Misc/Enums.h"
#include "PieceActor.h"
#include "JumperPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix
*/
UCLASS()
class DAVID_API AJumperPiece : public APieceActor
{
	GENERATED_BODY()

public:	
	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn() override;

private:
	bool bHasJumped = false;
};
