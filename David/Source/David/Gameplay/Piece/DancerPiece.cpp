#include "DancerPiece.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"

void ADancerPiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);

	OnPieceDestroyedOtherHandle = OnThisPieceDestroyedOtherDelegate.AddUObject(this, &ADancerPiece::Process_MoveHorizontal);
}

void ADancerPiece::ProcessTurn()
{
	bHasBeenProcessed = true;

	if (Square == nullptr) return;

	// Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 TargetSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

	if (!BoardManager->IsValidSquare(TargetSquareIndex)) return;

	if (BoardManager->IsSquareOccupied(TargetSquareIndex)) // Attack
	{
		Process_AttackPieceInSquare(TargetSquareIndex, EPieceAction::FrontAttack);
	}
	else // Move forward
	{
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
	}
}

void ADancerPiece::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	Super::OnPieceDestroyed(PieceInstigator);

	OnThisPieceDestroyedOtherDelegate.Remove(OnPieceDestroyedOtherHandle);
}

void ADancerPiece::Process_MoveHorizontal()
{
	if (Square == nullptr) return;

	// Get the target squares of the piece
	const int32 LeftIndex = Square->GetSquareIndex() - 1;
	const int32 RightIndex = Square->GetSquareIndex() + 1;
	const int32 PieceRow = Square->GetSquareIndex() / BoardManager->GetBoardWidth();

	const bool LeftAvailable = LeftIndex / BoardManager->GetBoardWidth() == PieceRow
		&& BoardManager->IsValidSquare(LeftIndex)
		&& !BoardManager->IsSquareOccupied(LeftIndex);
	const bool RightAvailable = RightIndex / BoardManager->GetBoardWidth() == PieceRow
		&& BoardManager->IsValidSquare(RightIndex)
		&& !BoardManager->IsSquareOccupied(RightIndex);

	if (RightAvailable && LeftAvailable) 
	{
		if (FMath::RandBool()) {
			Process_MoveToSquare(LeftIndex, EPieceAction::MoveToSquare);
		}
		else {
			Process_MoveToSquare(RightIndex, EPieceAction::MoveToSquare);
		}
	}
	else if (RightAvailable) 
	{
		Process_MoveToSquare(RightIndex, EPieceAction::MoveToSquare);
	}
	else if (LeftAvailable) 
	{
		Process_MoveToSquare(LeftIndex, EPieceAction::MoveToSquare);
	}
}
