#include "GraveDiggerPiece.h"
#include "../Board/BoardManager.h"
#include "../Board/BoardSquare.h"

void AGraveDiggerPiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);

	OnAnyBoardPieceDestroyedHandler = BoardManager->OnPieceDestroyed.AddUObject(this, &AGraveDiggerPiece::OnAnyBoardPieceDestroyed);
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
		LogPieceEvent(FString::Printf(TEXT("Piece destroyed, moving forward")));
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
		return;
	}

	LogPieceEvent(FString::Printf(TEXT("Piece destroyed, cannot moving forward")));
}
