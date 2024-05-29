#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "BoardSquare.generated.h"

UCLASS()
class DAVID_API ABoardSquare : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardSquare();

	/* Called during ProcessTurn() to change the players score */
	void Process_SetSquarePlayerColor(EDavidPlayer Player);

	/* Called during PlayAction() to change the square appeareance */
	void Action_SetSquarePlayerColor(EDavidPlayer Player);

	FORCEINLINE void SetBoardManager(class ABoardManager* BM) { BoardManager = BM; }

	FORCEINLINE ABoardManager* GetBoardManager() const { return BoardManager; }

	FORCEINLINE void SetPieceInSquare(class APieceActor* Piece) { PieceInSquare = Piece; }

	FORCEINLINE APieceActor* GetPieceInSquare() const { return PieceInSquare; }

	FORCEINLINE int32 GetSquareIndex() const { return SquareIndex; }

	FORCEINLINE EDavidSquareColor GetSquareColor() const { return ProcessSquareColor; }

	FORCEINLINE void SetIsLocked(bool IsLocked) { IsLocked = true; }

	FORCEINLINE bool IsLocked() const { return bIsLocked; }

private:
	void ChangeSquareColor();

private:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 SquareIndex;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UStaticMeshComponent* SquareMesh;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UMaterial* Player1SquareMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UMaterial* Player2SquareMaterial;
	
	UPROPERTY(Transient, SkipSerialization)
	ABoardManager* BoardManager;

	UPROPERTY(Transient, SkipSerialization)
	APieceActor* PieceInSquare;

	UPROPERTY(Transient, SkipSerialization)
	TEnumAsByte<EDavidSquareColor> SquareColor;
	
	/* Used only by the server to process the turn */
	UPROPERTY(Transient, SkipSerialization)
	TEnumAsByte<EDavidSquareColor> ProcessSquareColor;

	bool bIsLocked = false;
};
