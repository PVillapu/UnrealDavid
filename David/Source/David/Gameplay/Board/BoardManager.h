#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "TurnAction.h"
#include "BoardManager.generated.h"

class ABoardSquare;

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void Tick(float DeltaSeconds) override;

	void InitializeBoard();

	void PlayCardInSquare(struct FGameCardData& CardData, int32 SquareID, EDavidPlayer Player);

	void ProcessPlayerEndTurn(EDavidPlayer PlayerTurn);

	/* This is called when a piece is defeated */
	void OnPieceDeath(class APieceActor* PieceDestroyed, APieceActor* InstigatorPiece);

	/* Called during Process_ phase. Removes a piece from the ServerBoardPieces */
	void Process_RemovePieceFromLogicBoard(APieceActor* PieceToRemove);

	/* Must be called when a piece performs the last action in board before being destroyed */
	void RemoveActivePiece(APieceActor* Piece);

	FVector GetSquareLocation(int32 SquareIndex);

	bool CanPlayerPlayCardInSquare(EDavidPlayer Player, int32 SquareID);

	bool IsSquareOccupied(int32 TargetSquare) const;

	void MovePieceToSquare(APieceActor* Piece, int32 TargetSquare);

	void CheckIfAnyPieceFinished();

	void OnPieceReachedEndline(APieceActor* Piece);

	/* ---------------- Game Actions --------------------- */

	/* Registers an action that occurs in the processing phase of the turn */
	void RegisterGameAction(const FTurnAction& PieceAction);

	void PlayNextGameAction();

	void OnGameActionComplete();

	/* --------------------------------------------------- */

	/* ---------------- Aux methods ---------------------- */

	void CalculatePlayersScore(int32& OutPlayer1Score, int32& OutPlayer2Score);

	APieceActor* GetPieceInSquare(int32 BoardSquare) const;

	ABoardSquare* GetBoardSquare(int32 BoardIndex) const;

	FORCEINLINE bool IsValidSquare(int32 SquareIndex) const { return (SquareIndex >= 0 && SquareIndex < BoardSquares.Num()); }

	FORCEINLINE class AActor* GetPlayerCameraActor(int32 PlayerId) const { return PlayerId == 0 ? Player1Camera : Player2Camera; }

	FORCEINLINE int32 GetBoardHeight() const { return BoardHeight; }

	FORCEINLINE int32 GetBoardWidth() const { return BoardWidth; }

	/* --------------------------------------------------- */

	/* ---------------- Events delegates ---------------------- */

	/* Called when any piece in the board is destroyed */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerTurnChanged, APieceActor*, APieceActor*)
	FOnPlayerTurnChanged OnPieceDestroyed;

	/* --------------------------------------------------- */

private:
	void PlayPieceAction(const FTurnAction& TurnAction);

	void PlaySquareAction(const FTurnAction TurnAction);

	void PlayCardInSquareAction(const FTurnAction& TurnAction);

	APieceActor* InstantiateAndRegisterPiece(FGameCardData& GameCardData, const int32 SquareID, const int32 PieceID, const EDavidPlayer Player);

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_SendGameAction(FTurnAction TurnAction);

	FORCEINLINE int32 GetBoardIndex(int32 Row, int32 Col) const { return Row * BoardHeight + Col; }

private:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 BoardHeight = 6;

	UPROPERTY(EditAnywhere, Category = "David")
	int32 BoardWidth = 6;

	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<ABoardSquare> BoardSquareBP;

	UPROPERTY(EditAnywhere, Category = "David")
	AActor* Player1Camera;

	UPROPERTY(EditAnywhere, Category = "David")
	AActor* Player2Camera;

	// The instances of the board squares (must be sorted)
	UPROPERTY(EditAnywhere, Category = "David")
	TArray<ABoardSquare*> BoardSquares;

	/* This is used by the server to process the turn */
	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, APieceActor*> ServerBoardPieces;

	/* This is used by the clients to perform piece actions */
	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, APieceActor*> ActiveBoardPieces;

	/* Actions to play while the player is playing his current turn */
	UPROPERTY(Transient, SkipSerialization)
	TArray<FTurnAction> ActionsQueue;

	int32 PieceIdCounter;
	bool bProcessingAction;
};
