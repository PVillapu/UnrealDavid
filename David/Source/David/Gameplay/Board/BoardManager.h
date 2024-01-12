#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "../Piece/PieceAction.h"
#include "BoardManager.generated.h"

class ABoardSquare;

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void Tick(float DeltaSeconds) override;

	void GenerateBoardSquares();

	void PlayCardInSquare(struct FGameCardData& CardData, int32 SquareID, EDavidPlayer Player);

	void ProcessPlayerTurn(EDavidPlayer PlayerTurn);

	void SendTurnActions();

	void PlayTurnAction();

	void OnActionComplete();

	/* This is called during the turn process phase */
	void OnPieceDeathInTurnProcess(class APieceActor* Piece);

	/* Must be called when a piece performs the last action in board before being destroyed */
	void RemoveActivePiece(APieceActor* Piece);

	FVector GetSquareLocation(int32 SquareIndex);

	bool CanPlayerPlayCardInSquare(EDavidPlayer Player, int32 SquareID);

	void InitializeBoard();

	void RegisterTurnAction(const FPieceAction& PieceAction);

	bool IsSquareOccupied(int32 TargetSquare) const;

	void MovePieceToSquare(APieceActor* Piece, int32 TargetSquare);

	APieceActor* GetPieceInSquare(int32 BoardSquare) const;

	FORCEINLINE bool IsValidSquare(int32 SquareIndex) const { return (SquareIndex >= 0 && SquareIndex < BoardSquares.Num()); }

	FORCEINLINE class AActor* GetPlayerCameraActor(int32 PlayerId) const { return PlayerId == 0 ? Player1Camera : Player2Camera; }

	FORCEINLINE int32 GetBoardHeight() const { return BoardHeight; }

	FORCEINLINE int32 GetBoardWidth() const { return BoardWidth; }

private:

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_DeployPieceInSquare(FGameCardData CardData, int32 SquareID, int32 PieceID, EDavidPlayer Player);

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_SendTurnActions(const TArray<FPieceAction>& TurnActions);

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

	/* This is used by the server to process the turn */
	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, APieceActor*> ServerBoardPieces;

	/* This is used by the clients to perform piece actions */
	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, APieceActor*> ActiveBoardPieces;

	UPROPERTY()
	TArray<FPieceAction> TurnActionsQueue;

	int32 PieceIdCounter;
	bool bPlayNextAction;
};
