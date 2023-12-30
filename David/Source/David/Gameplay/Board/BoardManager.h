#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "BoardManager.generated.h"

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void GenerateBoardSquares();

	void PlayCardInSquare(const struct FCardData& CardData, int32 Square, int32 PieceID);

	void ProcessPlayerTurn(EDavidPlayer PlayerTurn);

	FORCEINLINE class AActor* GetPlayerCameraActor(int32 PlayerId) { return PlayerId == 0 ? Player1Camera : Player2Camera; }

protected:
	virtual void BeginPlay() override;

	void InitializeBoard();

private:

	FORCEINLINE int32 GetBoardIndex(int32 Row, int32 Col) const { return Row * BoardHeight + BoardWidth; }

protected:
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

	int32 PieceIdCounter;
};
