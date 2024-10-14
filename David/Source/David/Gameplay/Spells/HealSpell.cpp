#include "HealSpell.h"
#include "../Piece/PieceActor.h"
#include "../Board/BoardManager.h"

bool UHealSpell::CanSpellBePlayedInSquare(int32 SquareID, EDavidPlayer Player) const
{
    return BoardManager && BoardManager->IsSquareOccupied(SquareID);
}

void UHealSpell::PlaySpellAction(const FSpellAction &SpellAction)
{
	BoardManager->OnGameActionComplete();
}

void UHealSpell::Process_PlaySpell(int32 SquareID, EDavidPlayer Player)
{
	if(BoardManager && BoardManager->IsSquareOccupied(SquareID))
	{
		APieceActor* TargetPiece = BoardManager->GetPieceInSquare(SquareID);
		if(TargetPiece == nullptr) return;

		TargetPiece->Process_HealPiece(HealthAmmount);
	}
}
