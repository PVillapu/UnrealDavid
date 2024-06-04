#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/Enums.h"
#include "SquareAction.h"
#include "BoardSquare.generated.h"

UCLASS()
class DAVID_API ABoardSquare : public AActor
{
	GENERATED_BODY()
	
protected:
	enum EDavidSquareAction : int32
	{
		PAINT_SQUARE,
		LOCK_SQUARE
	};

public:	
	ABoardSquare();

	void ProcessAction(const FSquareAction& Action);

	/* Called during ProcessTurn() to change the players score */
	void Process_SetSquarePlayerColor(EDavidPlayer Player);

	/* Called during ProcessTurn() to lock the square */
	void Process_LockSquare();
	
	/* Called during PlayAction() to change the square appeareance */
	void Action_SetSquarePlayerColor(const FSquareAction& Action);

	void Action_LockSquare();

	FORCEINLINE void SetBoardManager(class ABoardManager* BM) { BoardManager = BM; }

	FORCEINLINE ABoardManager* GetBoardManager() const { return BoardManager; }

	FORCEINLINE void SetPieceInSquare(class APieceActor* Piece) { PieceInSquare = Piece; }

	FORCEINLINE APieceActor* GetPieceInSquare() const { return PieceInSquare; }

	FORCEINLINE int32 GetSquareIndex() const { return SquareIndex; }

	FORCEINLINE EDavidSquareColor GetSquareColor() const { return ProcessSquareColor; }

	FORCEINLINE bool IsLocked() const { return bIsLocked; }

	FORCEINLINE FVector GetSquarePieceLocation() const { return PieceBaseLocation->GetComponentLocation(); }

	/* Retrieves a SquareAction struct from the given Game Action */
	static FSquareAction GetSquareAction(const struct FTurnAction& GameAction);

protected:
	void RegisterSquareAction(int32 SquareAction);

	void RegisterSquareAction(int32 SquareAction, const TArray<uint8>& Payload);

private:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 SquareIndex;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UStaticMeshComponent* SquareMesh;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UMaterial* Player1SquareMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	UMaterial* Player2SquareMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	USceneComponent* PieceBaseLocation;
	
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
