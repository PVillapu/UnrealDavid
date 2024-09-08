#include "HorizontalAttacker.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"

void AHorizontalAttacker::ProcessTurn()
{
    bHasBeenProcessed = true;

    if (Square == nullptr) return;

    // Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
    const int32 ForwardSquare = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

    if(!SearchAndAttackArea() && !BoardManager->IsSquareOccupied(ForwardSquare))
    {
        Process_MoveToSquare(ForwardSquare, EPieceAction::MoveToSquare);
    }
}

bool AHorizontalAttacker::SearchAndAttackArea()
{
    bool HasAttacked = false;
    const int32 AttackRow = Square->GetSquareIndex() / BoardManager->GetBoardWidth();

    if(!BoardManager->IsValidSquare(Square->GetSquareIndex())) return false;

    TArray<int32> SquaresToAttack;

    for(int i = 1; i < BoardManager->GetBoardWidth(); ++i)
    {
        int32 HorizontalSquare = Square->GetSquareIndex() + i;
        if(HorizontalSquare / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(HorizontalSquare))
        {
            APieceActor* PieceInSquare = BoardManager->GetPieceInSquare(HorizontalSquare);
            if(!HasAttacked && PieceInSquare && PieceInSquare->GetOwnerPlayer() != DavidPlayerOwner)
            {
                HasAttacked = true;
            }
            
            SquaresToAttack.Add(HorizontalSquare);
        }
    }

    for(int i = 1; i < BoardManager->GetBoardWidth(); ++i)
    {
        int32 HorizontalSquare = Square->GetSquareIndex() - i;
        if(HorizontalSquare / BoardManager->GetBoardWidth() == AttackRow && BoardManager->IsValidSquare(HorizontalSquare))
        {
            APieceActor* PieceInSquare = BoardManager->GetPieceInSquare(HorizontalSquare);
            if(!HasAttacked && PieceInSquare && PieceInSquare->GetOwnerPlayer() != DavidPlayerOwner)
            {
                HasAttacked = true;
            }

            SquaresToAttack.Add(HorizontalSquare);
        }
    }

    if(HasAttacked)
    {
        Process_AttackPieces(SquaresToAttack);
    }

    return HasAttacked;
}