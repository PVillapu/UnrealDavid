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

	if (!SearchAndAttackEnemies(TargetSquareIndex) && !BoardManager->IsSquareOccupied(TargetSquareIndex))
	{
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
	}
}

bool AGuardPiece::SearchAndAttackEnemies(int32 FrontSquare)
{
	bool HasAttacked = false;
	const int32 AttackRow = FrontSquare / BoardManager->GetBoardWidth();
	TArray<int32> SquaresToAttack;

	if (BoardManager->IsValidSquare(FrontSquare) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		SquaresToAttack.Add(FrontSquare);
		HasAttacked = true;
	}

	if ((FrontSquare - 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare - 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner) 
	{
		SquaresToAttack.Add(FrontSquare - 1);
		HasAttacked = true;
	}

	if ((FrontSquare + 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare + 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		SquaresToAttack.Add(FrontSquare + 1);
		HasAttacked = true;
	}

	Process_AttackPiecesInSquares(SquaresToAttack);

	return HasAttacked;
}
