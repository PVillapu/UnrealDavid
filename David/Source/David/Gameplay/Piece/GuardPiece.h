#pragma once

#include "CoreMinimal.h"
#include "PieceActor.h"
#include "GuardPiece.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions nust have Action_ prefix

This piece can attack at his 3 front squares simultaneously

*/
UCLASS()
class DAVID_API AGuardPiece : public APieceActor
{
	GENERATED_BODY()

//public:	
//	AGuardPiece();
//
//	virtual void Tick(float DeltaSeconds) override;
//
//	/* Called when the piece is created by the BoardManager */
//	virtual void SetupPiece(class ABoardManager* BoardManagerActor, const struct FGameCardData& GameCardData, int32 ID, EDavidPlayer PieceOwner);
//
//	/* Called by the server when the piece needs to be processed */
//	virtual void ProcessTurn();
//
//	virtual void ProcessAction(const FPieceAction& Action);
//
//	virtual void OnBeginTurn();
//
//	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
//
//	virtual void OnPieceDestroyed(APieceActor* PieceInstigator);
//
//	/* Called when the piece has to be placed in the board */
//	virtual void OnDeployPieceInSquareAction(int32 SquareIndex);
//
//protected:
//	/* -------------------- Turn actions methods ------------------------ */
//
//	virtual void Action_MoveToSquare(const TArray<uint8>& Payload);
//
//	virtual void Action_AttackFrontPiece();
//
//	virtual void Action_TakeDamage(const TArray<uint8>& Payload);
//
//	virtual void Action_Die();
};
