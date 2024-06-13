#include "SpearThrowerPiece.h"
#include "../Board/BoardManager.h"
#include "../Board/BoardSquare.h"
#include "Kismet/GameplayStatics.h"

void ASpearThrowerPiece::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	Super::SetupPiece(BoardManagerActor, _GameCardData, ID, PieceOwner);

	bHasSpear = true;
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	
	for(int i = 1; i < BoardManager->GetBoardHeight(); ++i)
	{
		int32 NextSquare = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex * i;

		if (BoardManager->IsValidSquare(NextSquare) && BoardManager->IsSquareOccupied(NextSquare)) 
		{
			APieceActor* PieceInSquare = BoardManager->GetPieceInSquare(NextSquare);
			if (PieceInSquare->GetOwnerPlayer() == DavidPlayerOwner) continue;

			bHasSpear = false;

			// Throw the spear 
			TArray<uint8> Payload;
			Payload.SetNum(sizeof(int32));
			FMemory::Memcpy(Payload.GetData(), &NextSquare, sizeof(int32));
			RegisterPieceAction(ESpearPieceAction::ThrowSpear, Payload);

			// Deal spear damage
			UGameplayStatics::ApplyDamage(PieceInSquare, SpearDamage, nullptr, this, nullptr);

			break;
		}
	}
}

void ASpearThrowerPiece::ProcessTurn()
{
	bHasBeenProcessed = true;

	if (Square == nullptr) return;

	// Get the target square of the piece
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 ForwardSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;

	if (!BoardManager->IsValidSquare(ForwardSquareIndex)) return;

	APieceActor* EnemyPiece = nullptr;
	if (!AttackEnemyInRange() && !BoardManager->IsSquareOccupied(ForwardSquareIndex))
	{
		Process_MoveToSquare(ForwardSquareIndex, EPieceAction::MoveToSquare);
	}
}

void ASpearThrowerPiece::ProcessAction(const FPieceAction& Action)
{
	Super::ProcessAction(Action);
}

bool ASpearThrowerPiece::AttackEnemyInRange()
{
	const int32 FowardIndex = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? 1 : -1;
	const int32 ForwardSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex;
	const int32 ForwardSecondSquareIndex = Square->GetSquareIndex() + BoardManager->GetBoardWidth() * FowardIndex * 2;

	if (BoardManager->IsValidSquare(ForwardSquareIndex) && BoardManager->IsSquareOccupied(ForwardSquareIndex))
	{
		APieceActor* TargetPiece = BoardManager->GetPieceInSquare(ForwardSquareIndex);
		if (TargetPiece->GetOwnerPlayer() != DavidPlayerOwner) 
		{
			Process_AttackPieceInSquare(ForwardSquareIndex);
			return true;
		}
	}
	else if (BoardManager->IsValidSquare(ForwardSecondSquareIndex) && BoardManager->IsSquareOccupied(ForwardSecondSquareIndex))
	{
		APieceActor* TargetPiece = BoardManager->GetPieceInSquare(ForwardSecondSquareIndex);
		if (TargetPiece->GetOwnerPlayer() != DavidPlayerOwner)
		{
			Process_AttackPieceInSquare(ForwardSecondSquareIndex);
			return true;
		}
	}

	return false;
}
