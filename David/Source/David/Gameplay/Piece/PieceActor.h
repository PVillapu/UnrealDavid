#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceAction.h"
#include "PieceActor.generated.h"

UCLASS()
class DAVID_API APieceActor : public AActor
{
	GENERATED_BODY()
	
private:
	enum EPieceAction : int32 { MoveForward = 0, FrontAttack};

public:	
	APieceActor();

	/* Called when the piece is created by the BoardManager */
	virtual void SetupPiece(class ABoardManager* BoardManager, const struct FGameCardData& GameCardData, const struct FCardData& CardData, int32 ID);

	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

	virtual void ProcessActions(const FPieceAction& Action);

	/* Called by the server when the piece is played in the board */
	void DeployInSquare(int32 SquareIndex);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_DeployPieceInSquare(int32 SquareIndex);

	FORCEINLINE bool HasBeenProcessed() { return bHasBeenProcessed; }

protected:
	virtual void OnDeployPieceInSquare(int32 SquareIndex);

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(EPieceAction PieceAction) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	UPROPERTY(Transient, SkipSerialization)
	class ABoardManager* BoardManager;

	UPROPERTY(Transient, SkipSerialization)
	int32 BaseHealth;

	UPROPERTY(Transient, SkipSerialization)
	int32 BaseAttack;

	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentHealth;

	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentAttack;

	UPROPERTY(Transient, SkipSerialization)
	bool bHasBeenProcessed = false;

	int32 PieceID;
};
