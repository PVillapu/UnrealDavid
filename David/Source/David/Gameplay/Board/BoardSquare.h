#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardSquare.generated.h"

UCLASS()
class DAVID_API ABoardSquare : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardSquare();

	FORCEINLINE void SetBoardManager(class ABoardManager* BM) { BoardManager = BM; }

	FORCEINLINE ABoardManager* GetBoardManager() { return BoardManager; }

	FORCEINLINE void SetPieceInSquare(class APieceActor* Piece) { PieceInSquare = Piece; }

	FORCEINLINE APieceActor* GetPieceInSquare() const { return PieceInSquare; }

	FORCEINLINE void SetSquareIndex(int32 Index) { SquareIndex = Index; }

	FORCEINLINE int32 GetSquareIndex() { return SquareIndex; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	UStaticMeshComponent* SquareMesh;
	
	UPROPERTY(Transient, SkipSerialization)
	ABoardManager* BoardManager;

	UPROPERTY(Transient, SkipSerialization)
	APieceActor* PieceInSquare;

	int32 SquareIndex;
};
