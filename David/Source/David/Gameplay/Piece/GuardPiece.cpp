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

	LogPieceEvent(FString::Printf(TEXT("Searching for enemies | FrontSquare: %d"), FrontSquare));

	if (BoardManager->IsValidSquare(FrontSquare) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		LogPieceEvent(FString::Printf(TEXT("Found enemy in square index: %d"), FrontSquare));
		HasAttacked = true;
	}

	if ((FrontSquare - 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare - 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner) 
	{
		LogPieceEvent(FString::Printf(TEXT("Found enemy in square index: %d"), FrontSquare - 1));
		HasAttacked = true;
	}

	if ((FrontSquare + 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(FrontSquare + 1) && BoardManager->IsSquareOccupied(FrontSquare)
		&& BoardManager->GetBoardSquare(FrontSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
		LogPieceEvent(FString::Printf(TEXT("Found enemy in square index: %d"), FrontSquare - 1));
		HasAttacked = true;
	}

	LogPieceEvent(FString::Printf(TEXT("HasAttacked: %s"), HasAttacked ? "True" : "False"));

	if(HasAttacked)
	{
		SquaresToAttack.Add(FrontSquare);
		SquaresToAttack.Add(FrontSquare + 1);
		SquaresToAttack.Add(FrontSquare - 1);
		Process_AttackPieces(SquaresToAttack);
	}

	return HasAttacked;
}
