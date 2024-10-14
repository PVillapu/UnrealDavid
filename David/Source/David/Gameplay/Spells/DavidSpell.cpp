#include "DavidSpell.h"
#include "SpellAction.h"
#include "../Board/BoardManager.h"
#include "../Misc/Enums.h"

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

bool UDavidSpell::CanSpellBePlayedInSquare(int32 SquareID, EDavidPlayer Player) const
{
    return false;
}

void UDavidSpell::PlaySpellAction(const FSpellAction& SpellAction)
{
#if WITH_EDITOR

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("NOT IMPLEMENTED! -- UDavidSpell::PlaySpellAction | SpellID: %d, SquareID: %d"), SpellAction.SpellID, SpellAction.SquareID);
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(Message)
		);
	}

#endif

	BoardManager->OnGameActionComplete();
}

void UDavidSpell::Process_PlaySpell(int32 SquareID, EDavidPlayer Player)
{
#if WITH_EDITOR

	if (GEngine)
	{
		FString Message = FString::Printf(TEXT("NOT IMPLEMENTED! -- UDavidSpell::Process_PlaySpell | SquareID: %d, Player: %d"), SquareID, (int32)Player);
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(Message)
		);
	}

#endif
}
