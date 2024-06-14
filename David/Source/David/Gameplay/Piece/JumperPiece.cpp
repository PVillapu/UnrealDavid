#include "JumperPiece.h"
#include "../Board/BoardManager.h"
#include "../Board/BoardSquare.h"

void AJumperPiece::ProcessTurn()
{
	bHasBeenProcessed = true;

	if (Square == nullptr) return;

	// Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 FrontSquare = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;
	const int32 JumpSquareTarget = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex * 2;

	// If the jump target square is not ocuppied, jump to it
	if (!bHasJumped && BoardManager->IsValidSquare(JumpSquareTarget))
	{
		// Jump square not occupied
		if (!BoardManager->IsSquareOccupied(JumpSquareTarget)) 
		{
			Process_MoveToSquare(JumpSquareTarget, EPieceAction::MoveToSquare);
			bHasJumped = true;
			return;
		}
	}

	// If the front square is available, move to it
	if (BoardManager->IsValidSquare(FrontSquare))
	{
		if (!BoardManager->IsSquareOccupied(FrontSquare))
		{
			Process_MoveToSquare(FrontSquare, EPieceAction::MoveToSquare);
			return;
		}
	}
	else return;

	const APieceActor* TargetPiece = BoardManager->GetPieceInSquare(FrontSquare);

	// Attack the front piece
	if (TargetPiece && !IsPieceOfSameTeam(TargetPiece)) // Attack
	{
		Process_AttackPiece(FrontSquare);
	}
}
