#include "DiagonalAttackerPiece.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"

void ADiagonalAttackerPiece::ProcessTurn()
{
	bHasBeenProcessed = true;

	if (Square == nullptr) return;

	if (!Process_AttackPieceInDiagonals())
	{
		// Get the target square of the piece
		const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
		const int32 TargetSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

		if (!BoardManager->IsValidSquare(TargetSquareIndex)) return;

		if (!BoardManager->IsSquareOccupied(TargetSquareIndex))
		{
			Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
		}
	}
}

bool ADiagonalAttackerPiece::Process_AttackPieceInDiagonals()
{
	TArray<int32> SquaresToAttack;

	if (SearchDiagonalTargets(Square->GetSquareIndex(), true, SquaresToAttack) || SearchDiagonalTargets(Square->GetSquareIndex(), false, SquaresToAttack)) 
	{
		for (int32 Target : SquaresToAttack) 
		{
			Process_AttackPiece(Target);
		}
		return true;
	}

	return false;
}

bool ADiagonalAttackerPiece::SearchDiagonalTargets(int32 TargetSquare, bool LeftSide, TArray<int32>& SquaresToAttack)
{
	// TODO
	LogPieceEvent(FString::Printf(TEXT("TODO implementation")));
	return false;
}
