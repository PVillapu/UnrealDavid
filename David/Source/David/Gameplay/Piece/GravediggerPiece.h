#pragma once

#include "CoreMinimal.h"
#include "../Misc/Enums.h"
#include "PieceAction.h"
#include "PieceActor.h"
#include "GraveDiggerPiece.generated.h"

UCLASS()
class DAVID_API AGraveDiggerPiece : public APieceActor
{
	GENERATED_BODY()

public:
	//virtual void Tick(float DeltaSeconds) override;

	/* Called when the piece is created by the BoardManager */
	void SetupPiece(class ABoardManager* BoardManagerActor, const struct FGameCardData& GameCardData, const struct FCardData& CardData, int32 ID, EDavidPlayer PieceOwner) override;

	void ProcessAction(const FPieceAction& Action) override;

	void OnPieceDestroyed(APieceActor* PieceInstigator) override;

private:
	void OnAnyBoardPieceDestroyed(APieceActor* DestroyedPiece, APieceActor* InstigatorPiece);

private:

	FDelegateHandle OnAnyBoardPieceDestroyedHandler;
};
