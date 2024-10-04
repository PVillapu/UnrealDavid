#pragma once

#include "CoreMinimal.h"
#include "DavidSpell.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions must have Action_ prefix
*/
UCLASS()
class DAVID_API UDavidSpell : public UObject
{
	GENERATED_BODY()

public:

	void SetupSpell(class ABoardManager* BoardManager);

	/* Returns a spell action struct from a given turn action */
	static struct FSpellAction GetSpellAction(const struct FTurnAction& TurnAction);

	virtual bool CanSpellBePlayedInSquare(int32 SquareID, enum EDavidPlayer Player) const;

	virtual void PlaySpellAction(const FSpellAction& SpellAction);

	virtual void Process_PlaySpell(int32 SquareID, enum EDavidPlayer Player);

protected:
	UPROPERTY(Transient, SkipSerialization)
	ABoardManager* BoardManager;
};