#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "DancerPiece.generated.h"

UCLASS()
class DAVID_API ADancerPiece : public APieceActor
{
	GENERATED_BODY()

public:	
	void SetupPiece(class ABoardManager* BoardManagerActor, struct FGameCardData& GameCardData, FCardData& CardData, int32 ID, EDavidPlayer PieceOwner) override;
	
	/* Called by the server when the piece needs to be processed */
	void ProcessTurn() override;

	void OnPieceDestroyed(APieceActor* PieceInstigator) override;

private:
	void Process_MoveHorizontal();

private:
	FDelegateHandle OnPieceDestroyedOtherHandle;
};
