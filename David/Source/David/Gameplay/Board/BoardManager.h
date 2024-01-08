#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "../Piece/PieceAction.h"
#include "Containers/Queue.h"
#include "BoardManager.generated.h"

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void GenerateBoardSquares();

	void PlayCardInSquare(struct FGameCardData& CardData, int32 SquareID, EDavidPlayer Player);

	void ProcessPlayerTurn(EDavidPlayer PlayerTurn);

	void PlayTurnActions();

	FVector GetSquareLocation(int32 SquareIndex);

	bool CanPlayerPlayCardInSquare(EDavidPlayer Player, int32 SquareID);

	FORCEINLINE class AActor* GetPlayerCameraActor(int32 PlayerId) { return PlayerId == 0 ? Player1Camera : Player2Camera; }

	void InitializeBoard();

	void AddTurnAction(FPieceAction* PieceAction);

private:

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_DeployPieceInSquare(FGameCardData CardData, int32 SquareID, int32 PieceID, EDavidPlayer Player);

	FORCEINLINE int32 GetBoardIndex(int32 Row, int32 Col) const { return Row * BoardHeight + Col; }

private:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 BoardHeight = 6;

	UPROPERTY(EditAnywhere, Category = "David")
	int32 BoardWidth = 6;

	UPROPERTY(EditAnywhere, Category = "David")
	float SquaresOffset = 2.5f;

	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<class ABoardSquare> BoardSquareBP;

	UPROPERTY(EditAnywhere, Category = "David")
	AActor* Player1Camera;

	UPROPERTY(EditAnywhere, Category = "David")
	AActor* Player2Camera;

	// Just the board squares (provisional)
	UPROPERTY(Transient, SkipSerialization)
	TArray<ABoardSquare*> BoardSquares;

	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, class APieceActor*> BoardPieces;

	TQueue<FPieceAction*> TurnActionsQueue;

	int32 PieceIdCounter;
};
