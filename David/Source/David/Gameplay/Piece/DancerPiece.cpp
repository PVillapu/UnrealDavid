#include "DancerPiece.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"

void ADancerPiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);

	OnPieceDestroyedOtherHandle = OnThisPieceDestroyedOtherDelegate.AddUObject(this, &ADancerPiece::Process_MoveHorizontal);
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
		if (FMath::RandBool()) 
		{
    		LogPieceEvent(FString::Printf(TEXT("Moved to left square: %d"), LeftIndex));
			Process_MoveToSquare(LeftIndex, EPieceAction::MoveToSquare);
		}
		else 
		{
			LogPieceEvent(FString::Printf(TEXT("Moved to right square: %d"), RightIndex));
			Process_MoveToSquare(RightIndex, EPieceAction::MoveToSquare);
		}
	}
	else if (RightAvailable) 
	{
		LogPieceEvent(FString::Printf(TEXT("Moved to right square: %d"), RightIndex));
		Process_MoveToSquare(RightIndex, EPieceAction::MoveToSquare);
	}
	else if (LeftAvailable) 
	{
		LogPieceEvent(FString::Printf(TEXT("Moved to right square: %d"), LeftIndex));
		Process_MoveToSquare(LeftIndex, EPieceAction::MoveToSquare);
	}
}
