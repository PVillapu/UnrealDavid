#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceAction.h"
#include "../Misc/Enums.h"
#include "PieceActor.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix
*/
UCLASS()
class DAVID_API APieceActor : public AActor
{
	GENERATED_BODY()
	
private:
	enum EPieceAction : int32 { MoveForward = 0, FrontAttack, TakePieceDamage, Die};

public:	
	APieceActor();

	void Tick(float DeltaSeconds) override;

	/* Called when the piece is created by the BoardManager */
	virtual void SetupPiece(class ABoardManager* BoardManager, const struct FGameCardData& GameCardData, const struct FCardData& CardData, int32 ID, EDavidPlayer PieceOwner);

	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

	virtual void ProcessAction(const FPieceAction& Action);

	virtual void OnBeginTurn();

	/* Called by the server when the piece is played in the board */
	void DeployInSquare(int32 SquareIndex);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_DeployPieceInSquare(int32 SquareIndex);

	/* Returns true if this piece has been processed in this turn */
	FORCEINLINE bool HasBeenProcessed() const { return bHasBeenProcessed; }

	/* Return the player that own this piece */
	FORCEINLINE EDavidPlayer GetOwnerPlayer() const { return DavidPlayerOwner;	}

	/* Sets the current square of this piece */
	FORCEINLINE void SetBoardSquare(class ABoardSquare* SquareActor) { Square = SquareActor; }

	/* Gets the current square of this piece */
	FORCEINLINE ABoardSquare* GetBoardSquare() const { return Square; }

	FORCEINLINE int32 GetPieceID() const { return PieceID; }

protected:
	virtual void OnDeployPieceInSquare(int32 SquareIndex);

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(EPieceAction PieceAction) const;

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(EPieceAction PieceAction, TArray<uint8>& Payload) const;

	/* -------------------- Turn process methods ----------------------- */

	void Process_MoveForward();

	void Process_AttackFrontPiece();

	/* -------------------- Turn actions methods ------------------------ */

	virtual void Action_MoveForward(const TArray<uint8>& Payload);

	virtual void Action_AttackFrontPiece();

	virtual void Action_TakeDamage(const TArray<uint8>& Payload);

	virtual void Action_Die();

	void HandlePieceMovement(float DeltaSeconds);

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

	EDavidPlayer DavidPlayerOwner;

	UPROPERTY(Transient, SkipSerialization)
	ABoardSquare* Square;
	
	int32 PieceID;

	/* ---------- Movement variables  ---------------- */

	UPROPERTY(Transient, SkipSerialization)
	FVector TargetLocation;

	UPROPERTY(Transient, SkipSerialization)
	FVector OriginLocation;

	/* Time that takes the movement to reach the destination */
	float MovementTime = 0.7f;

	/* Stores the time since movement started */
	float MovementDelta = 0.f;

	bool bIsMoving = false;
};
