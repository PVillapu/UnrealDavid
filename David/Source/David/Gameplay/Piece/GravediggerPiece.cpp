#include "GraveDiggerPiece.h"
#include "../Board/BoardManager.h"
#include "../Board/BoardSquare.h"

void AGraveDiggerPiece::SetupPiece(ABoardManager* BoardManagerActor, const FGameCardData& GameCardData, FCardData& _CardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, GameCardData, _CardData, ID, PieceOwner);

	OnAnyBoardPieceDestroyedHandler = BoardManager->OnPieceDestroyed.AddUObject(this, &AGraveDiggerPiece::OnAnyBoardPieceDestroyed);
}

void AGraveDiggerPiece::ProcessAction(const FPieceAction& Action)
{
	Super::ProcessAction(Action);
}

void AGraveDiggerPiece::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	Super::OnPieceDestroyed(PieceInstigator);

	BoardManager->OnPieceDestroyed.Remove(OnAnyBoardPieceDestroyedHandler);
}

void AGraveDiggerPiece::OnAnyBoardPieceDestroyed(APieceActor* DestroyedPiece, APieceActor* InstigatorPiece)
{
	if (Square == nullptr) return;

	// Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 TargetSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

	if (!BoardManager->IsValidSquare(TargetSquareIndex)) return;

	if (!BoardManager->IsSquareOccupied(TargetSquareIndex)) // Move forward
	{
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveForward);
	}
}
