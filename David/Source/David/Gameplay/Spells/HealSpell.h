#pragma once

#include "CoreMinimal.h"
#include "DavidSpell.h"
#include "HealSpell.generated.h"

/* 
Piece actor, all methods that correspond to the turn processing must have Process_ prefix,
and all methods used to play actions must have Action_ prefix
*/
UCLASS(Blueprintable, BlueprintType)
class DAVID_API UHealSpell : public UDavidSpell
{
	GENERATED_BODY()

public:

	virtual bool CanSpellBePlayedInSquare(int32 SquareID, enum EDavidPlayer Player) const override;

	virtual void PlaySpellAction(const FSpellAction& SpellAction) override;

	virtual void Process_PlaySpell(int32 SquareID, enum EDavidPlayer Player) override;

private:
	UPROPERTY(EditDefaultsOnly)
	int32 HealthAmmount = 2;
};