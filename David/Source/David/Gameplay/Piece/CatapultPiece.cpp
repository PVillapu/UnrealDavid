#include "CatapultPiece.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"

void ACatapultPiece::ProcessTurn()
{
    bHasBeenProcessed = true;

    if (Square == nullptr) return;

    // Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 TargetMiddleSquare = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * AttackRange * FowardIndex;

    if(!SearchAndAttackArea(TargetMiddleSquare) && !BoardManager->IsSquareOccupied(TargetMiddleSquare))
    {
        const int32 ForwardSquare = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;
        Process_MoveToSquare(ForwardSquare, EPieceAction::MoveToSquare);
    }
}

bool ACatapultPiece::SearchAndAttackArea(int32 TargetMiddleSquare)
{
    bool HasAttacked = false;
	const int32 AttackRow = TargetMiddleSquare / BoardManager->GetBoardWidth();
    LogPieceEvent(FString::Printf(TEXT("Searching enemies | TargetMiddleSquare: %d"), TargetMiddleSquare));

    if(!BoardManager->IsValidSquare(TargetMiddleSquare)) return false;

	TArray<int32> SquaresToAttack;

    // Mid square
	if (BoardManager->IsSquareOccupied(TargetMiddleSquare)
		&& BoardManager->GetBoardSquare(TargetMiddleSquare)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner)
	{
        LogPieceEvent(FString::Printf(TEXT("Enemy found in mid square | Index: %d"), TargetMiddleSquare));
		HasAttacked = true;
	}

    // Side square 1
	if (!HasAttacked && (TargetMiddleSquare - 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(TargetMiddleSquare - 1))
    {
        LogPieceEvent(FString::Printf(TEXT("Enemy found in mid square | Index: %d"), (TargetMiddleSquare - 1)));
        HasAttacked = BoardManager->IsSquareOccupied(TargetMiddleSquare - 1) && BoardManager->GetBoardSquare(TargetMiddleSquare - 1)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner; 
    } 
        
    // Side square 2
	if (!HasAttacked && (TargetMiddleSquare + 1) / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(TargetMiddleSquare + 1))
	{
        LogPieceEvent(FString::Printf(TEXT("Enemy found in mid square | Index: %d"), (TargetMiddleSquare + 1)));
		HasAttacked = BoardManager->IsSquareOccupied(TargetMiddleSquare + 1) && BoardManager->GetBoardSquare(TargetMiddleSquare + 1)->GetPieceInSquare()->GetOwnerPlayer() != DavidPlayerOwner;
	}

    LogPieceEvent(FString::Printf(TEXT("Has attacked: %s"), HasAttacked ? "True" : "False"));

    if(HasAttacked)
    {
        SquaresToAttack.Add(TargetMiddleSquare);
        SquaresToAttack.Add(TargetMiddleSquare + 1);
        SquaresToAttack.Add(TargetMiddleSquare - 1);
	    Process_AttackPieces(SquaresToAttack);
    }

	return HasAttacked;
}
