#include "DavidSpell.h"
#include "SpellAction.h"
#include "../Board/BoardManager.h"

void UDavidSpell::SetupSpell(ABoardManager* _BoardManager)
{
	BoardManager = _BoardManager;
}

FSpellAction UDavidSpell::GetSpellAction(const FTurnAction &TurnAction)
{
	int32 SpellID;
	int32 SquareID;
	TArray<uint8> Payload;
	Payload.SetNum(TurnAction.Payload.Num() - 2 * sizeof(int32));

	FMemory::Memcpy(&SpellID, TurnAction.Payload.GetData(), sizeof(int32));
	FMemory::Memcpy(&SquareID, &TurnAction.Payload[4], sizeof(int32));

	// Retrieve payload only if there is one available
	if(Payload.Num() != 0)
		FMemory::Memcpy(Payload.GetData(), &TurnAction.Payload[8], Payload.Num());

    return FSpellAction(SpellID, SquareID, Payload);
}

void UDavidSpell::ProcessSpellAction(const FSpellAction& SpellAction)
{
#if WITH_EDITOR

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("NOT IMPLEMENTED! -- UDavidSpell::ProcessSpellAction | SpellID: %d, SquareID: %d"), SpellAction.SpellID, SpellAction.SquareID);
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString(Message)
		);
	}

#endif

	BoardManager->OnGameActionComplete();
}
