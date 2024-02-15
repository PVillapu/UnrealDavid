#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceAction.h"
#include "../Board/TurnAction.h"
#include "../Misc/Enums.h"
#include "../Cards/CardData.h"
#include "PieceActor.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix
*/
UCLASS()
class DAVID_API APieceActor : public AActor
{
	GENERATED_BODY()
	
protected:
	enum EPieceAction : int32 { MoveForward = 0, FrontAttack, TakePieceDamage, Die};

public:	
	APieceActor();

	virtual void Tick(float DeltaSeconds) override;

	/* Called when the piece is created by the BoardManager */
	virtual void SetupPiece(class ABoardManager* BoardManagerActor, const struct FGameCardData& GameCardData, FCardData& CardData, int32 ID, EDavidPlayer PieceOwner);

	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

	virtual void ProcessAction(const FPieceAction& Action);

	virtual void OnBeginTurn();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPieceDestroyed(APieceActor* PieceInstigator);

	/* Called when the piece has to be placed in the board */
	virtual void OnDeployPieceInSquareAction(int32 SquareIndex);

	/* Returns true if this piece has been processed in this turn */
	FORCEINLINE bool HasBeenProcessed() const { return bHasBeenProcessed; }

	/* Return the player that own this piece */
	FORCEINLINE EDavidPlayer GetOwnerPlayer() const { return DavidPlayerOwner;	}

	/* Sets the current square of this piece */
	FORCEINLINE void SetBoardSquare(class ABoardSquare* SquareActor) { Square = SquareActor; }

	/* Gets the current square of this piece */
	FORCEINLINE ABoardSquare* GetBoardSquare() const { return Square; }

	FORCEINLINE int32 GetPieceID() const { return PieceID; }

	FORCEINLINE FCardData GetCardData() const { return CardData; }

	FORCEINLINE int32 GetPieceAttack() const { return CurrentAttack; }

	FORCEINLINE int32 GetPieceHealth() const { return CurrentHealth; }

	/* Retrieves a PieceAction struct from the given Game Action */
	static FPieceAction GetPieceAction(const FTurnAction& GameAction);

protected:
	UFUNCTION()
	virtual void OnBeginCursorOverEvent(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	virtual void OnEndCursorOverEvent(UPrimitiveComponent* TouchedComponent);

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(int32 PieceAction) const;

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(int32 PieceAction, TArray<uint8>& Payload) const;

	/* -------------------- Turn process methods ----------------------- */

	void Process_MoveToSquare(const int32& TargetSquareIndex, int32 ActionID);

	void Process_AttackPieceInSquare(const int32& TargetSquareIndex, int32 ActionID);

	/* -------------------- Turn actions methods ------------------------ */

	virtual void Action_MoveToSquare(const TArray<uint8>& Payload);

	virtual void Action_AttackFrontPiece();

	virtual void Action_TakeDamage(const TArray<uint8>& Payload);

	virtual void Action_Die();

	void HandlePieceMovement(float DeltaSeconds);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	USkeletalMeshComponent* SkeletalMeshComponent;

	/*UPROPERTY(EditDefaultsOnly, Category = "David")
	class UWidgetComponent* StatsWidgetComponent;*/
	
	UPROPERTY(Transient, SkipSerialization)
	class ABoardManager* BoardManager;

	UPROPERTY(Transient, SkipSerialization)
	int32 BaseHealth;

	UPROPERTY(Transient, SkipSerialization)
	int32 BaseAttack;

	UPROPERTY(Transient, SkipSerialization)
	int32 ProcessHealth;

	UPROPERTY(Transient, SkipSerialization)
	int32 ProcessAttack;

	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentHealth;

	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentAttack;

	UPROPERTY(Transient, SkipSerialization)
	bool bHasBeenProcessed = false;

	UPROPERTY(Transient, SkipSerialization)
	ABoardSquare* Square;

	UPROPERTY(Transient, SkipSerialization)
	class UGameHUD* GameHUD;

	UPROPERTY(Transient, SkipSerialization)
	FCardData CardData;

	EDavidPlayer DavidPlayerOwner;
	
	int32 PieceID;

	/* ---------- Movement variables  ---------------- */

	UPROPERTY(Transient, SkipSerialization)
	FVector TargetLocation;

	UPROPERTY(Transient, SkipSerialization)
	FVector OriginLocation;

	UPROPERTY(Transient, SkipSerialization)
	ABoardSquare* TargetSquare;

	/* Time that takes the movement to reach the destination */
	float MovementTime = 0.7f;

	/* Stores the time since movement started */
	float MovementDelta = 0.f;

	bool bIsMoving = false;
};
