#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceAction.h"
#include "../Board/TurnAction.h"
#include "../Misc/Enums.h"
#include "../Cards/GameCardData.h"
#include "PieceActor.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions must have Action_ prefix
*/
UCLASS()
class DAVID_API APieceActor : public AActor
{
	GENERATED_BODY()
	
protected:
	enum EPieceAction : int32 { MoveToSquare = 0, FrontAttack, TakePieceDamage, Die, ReachedEndSquare, MAX_VALUE};

public:	
	APieceActor();

	virtual void Tick(float DeltaSeconds) override;

	/* Called when the piece is created by the BoardManager */
	virtual void SetupPiece(class ABoardManager* BoardManagerActor, FGameCardData& GameCardData, int32 ID, EDavidPlayer PieceOwner);

	/* Called by the server when the piece needs to be processed */
	virtual void ProcessTurn();

	virtual void ProcessAction(const FPieceAction& Action);

	virtual void OnBeginTurn();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPieceDestroyed(APieceActor* PieceInstigator);

	virtual void OnPieceReachedEndLine();

	/* Called when the piece has to be placed in the board */
	virtual void OnDeployPieceInSquareAction(int32 SquareIndex);

	virtual FGameCardData GetDeathCard() const;

	/* Returns true if this piece has been processed in this turn */
	FORCEINLINE bool HasBeenProcessed() const { return bHasBeenProcessed; }

	/* Return the player that own this piece */
	FORCEINLINE EDavidPlayer GetOwnerPlayer() const { return DavidPlayerOwner;	}

	/* Sets the current square of this piece */
	FORCEINLINE void SetBoardSquare(class ABoardSquare* SquareActor) { Square = SquareActor; }

	/* Gets the current square of this piece */
	FORCEINLINE ABoardSquare* GetBoardSquare() const { return Square; }

	FORCEINLINE int32 GetPieceID() const { return PieceID; }

	FORCEINLINE int32 GetCardDataIndex() const { return GameCardData.CardDTIndex; }

	FORCEINLINE int32 GetPieceAttack() const { return CurrentAttack; }

	FORCEINLINE int32 GetPieceHealth() const { return CurrentHealth; }

	/* Retrieves a PieceAction struct from the given Game Action */
	static FPieceAction GetPieceAction(const FTurnAction& GameAction);

protected:
	/* ----------------------- Piece delegates ------------------------- */

	DECLARE_MULTICAST_DELEGATE(FOnThisPieceDestroyedOther)
	FOnThisPieceDestroyedOther OnThisPieceDestroyedOtherDelegate;

	FORCEINLINE void OnThisPieceDestroyedOther() { OnThisPieceDestroyedOtherDelegate.Broadcast(); }

	/*------------------------------------------------------------------ */

	UFUNCTION()
	virtual void OnBeginCursorOverEvent(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	virtual void OnEndCursorOverEvent(UPrimitiveComponent* TouchedComponent);

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(int32 PieceAction) const;

	/* Registers a PieceAction in the board to later play it */
	void RegisterPieceAction(int32 PieceAction, const TArray<uint8>& Payload) const;

	/* -------------------- Turn process methods ----------------------- */

	void Process_MoveToSquare(const int32 TargetSquareIndex, const int32 ActionID);

	void Process_AttackPieceInSquare(const int32 TargetSquareIndex, const int32 ActionID);

	/* -------------------- Turn actions methods ------------------------ */

	virtual void Action_MoveToSquare(const TArray<uint8>& Payload);

	virtual void Action_AttackFrontPiece();

	virtual void Action_TakeDamage(const TArray<uint8>& Payload);

	virtual void Action_Die();

	virtual void Action_ReachedEndLine();

	void HandlePieceMovement(float DeltaSeconds);

	void HandlePieceAttack(float DeltaSeconds);

	void HandlePieceReceiveDamage(float DeltaSeconds);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	class UWidgetComponent* StatsWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	class USceneComponent* PieceRoot;

	/* Vector curve used to move the piece to target square
	 * NOTE: The Y axis of the curve is clamped between 0 and 1 to represent the start and end position of the movement,
	 * rest of axis are absolute offset values
	 */
	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|Movement")
	class UCurveVector* PieceMovementCurve;
	
	/* Vector curve used to set the piece offset while is attacking */
	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|Attacking")
	class UCurveVector* PieceAttackCurve;

	/* Vector curve used to set the piece offset when receives an attack */
	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|RecivingDamage")
	class UCurveVector* PieceReceiveDamageCurvePosition;

	/* Vector curve used to set the piece offset when receives an attack */
	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|RecivingDamage")
	class UCurveVector* PieceReceiveDamageCurveRotation;

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
	FGameCardData GameCardData;

	UPROPERTY(Transient, SkipSerialization)
	class UPieceStats* StatsWidget;

	EDavidPlayer DavidPlayerOwner;
	
	int32 PieceID;

	/* -------------- Movement variables  ------------------- */

	bool bIsMoving = false;

	UPROPERTY(Transient, SkipSerialization)
	FVector Action_TargetLocation;

	UPROPERTY(Transient, SkipSerialization)
	FVector Action_OriginLocation;

	UPROPERTY(Transient, SkipSerialization)
	ABoardSquare* Action_TargetSquare;

	/* Time that takes the movement to reach the destination */
	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|Movement")
	float MovementTime = 0.8f;

	/* Stores the time since movement started */
	float MovementDelta = 0.f;

	/* -------------- Attacking variables ------------------ */

	bool bIsAttacking = false;

	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|Attacking")
	float AttackTime = 0.8f;

	float AttackDelta = 0.f;

	/* -------------- Reciving damage variables ------------- */

	bool bIsReceivingDamage = false;

	UPROPERTY(EditDefaultsOnly, Category = "David|Configuration|RecivingDamage")
	float ReceiveDamageTime = 0.8f;

	float ReceiveDamageDelta = 0.f;
};
