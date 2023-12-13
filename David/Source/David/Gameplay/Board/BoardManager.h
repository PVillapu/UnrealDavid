#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardManager.generated.h"

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void GenerateBoardSquares();

	void PlayCardInSquare(const struct FCardData& CardData, int32 Col, int32 Row);

protected:
	virtual void BeginPlay() override;

	void InitializeBoard();

private:

	FORCEINLINE int GetBoardIndex(int Row, int Col) const { return Row * BoardHeight + BoardWidth; }

protected:
	UPROPERTY(EditAnywhere, Category = "Board")
	int BoardHeight = 6;

	UPROPERTY(EditAnywhere, Category = "Board")
	int BoardWidth = 6;

	UPROPERTY(EditAnywhere, Category = "Board")
	float SquaresOffset = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Board")
	TSubclassOf<AActor> BoardSquareBP;

	// Just the board squares (provisional)
	UPROPERTY(Transient, SkipSerialization)
	TArray<AActor*> BoardSquares;

	UPROPERTY(Transient, SkipSerialization)
	TArray<int32> Board;

	UPROPERTY(Transient, SkipSerialization)
	TMap<int32, class APieceActor*> BoardPieces;

	int32 PieceIdCounter;
};
