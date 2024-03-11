#include "GuardPiece.h"
#include "../Board/BoardManager.h"
#include "../Board/BoardSquare.h"

void AGuardPiece::ProcessTurn()
{
	bHasBeenProcessed = true;

	if (Square == nullptr) return;

	// Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 TargetSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

	if (!BoardManager->IsValidSquare(TargetSquareIndex)) return;

	if (!SearchAndAttackEnemies(TargetSquareIndex))
	{
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
	}
}

bool AGuardPiece::SearchAndAttackEnemies(const int32 FrontSquare)
{
	bool HasAttacked = false;
	const int32 AttackRow = FrontSquare / BoardManager->GetBoardWidth();

	if (BoardManager->IsValidSquare(FrontSquare) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		Process_AttackPieceInSquare(FrontSquare, EPieceAction::FrontAttack);
		HasAttacked = true;
	}

	if ((FrontSquare - 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare - 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner) 
	{
		Process_AttackPieceInSquare(FrontSquare - 1, EPieceAction::FrontAttack);
		HasAttacked = true;
	}

	if ((FrontSquare + 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare + 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		Process_AttackPieceInSquare(FrontSquare + 1, EPieceAction::FrontAttack);
		HasAttacked = true;
	}

	return HasAttacked;
}
