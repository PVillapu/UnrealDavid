#include "PieceActor.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "../Cards/CardData.h"
#include "Kismet/GameplayStatics.h"
#include "../Board/BoardSquare.h"
#include "../Misc/CustomDavidLogs.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void APieceActor::Tick(float DeltaSeconds)
{
	if (bIsMoving) 
	{
		HandlePieceMovement(DeltaSeconds);
	}
}

void APieceActor::SetupPiece(ABoardManager* BoardManagerActor, const FGameCardData& GameCardData, const FCardData& CardData, int32 ID, EDavidPlayer PieceOwner)
{
	BoardManager = BoardManagerActor;

	CurrentHealth = BaseHealth = GameCardData.PieceHealth;
	CurrentAttack = BaseAttack = GameCardData.PieceAttack;

	PieceID = ID;
	DavidPlayerOwner = PieceOwner;
}

void APieceActor::OnBeginTurn()
{
	bHasBeenProcessed = false;
}

void APieceActor::RegisterPieceAction(int32 PieceAction) const
{
	FTurnAction GameAction;
	GameAction.ActionType = EDavidGameAction::PIECE_ACTION;
	GameAction.Payload.SetNum(2 * sizeof(int32));
	FMemory::Memcpy(GameAction.Payload.GetData(), &PieceID, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[4], &PieceAction, sizeof(int32));

	BoardManager->RegisterGameAction(GameAction);
}

void APieceActor::RegisterPieceAction(int32 PieceAction, TArray<uint8>& Payload) const
{
	FTurnAction GameAction;
	GameAction.ActionType = EDavidGameAction::PIECE_ACTION;
	GameAction.Payload.SetNum(2 * sizeof(int32) + Payload.Num());
	FMemory::Memcpy(GameAction.Payload.GetData(), &PieceID, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[4], &PieceAction, sizeof(int32));
	FMemory::Memcpy(&GameAction.Payload[8], Payload.GetData(), Payload.Num());

	BoardManager->RegisterGameAction(GameAction);
}

float APieceActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TArray<uint8> Payload;
	Payload.SetNum(sizeof(float));
	FMemory::Memcpy(Payload.GetData(), &DamageAmount, sizeof(float));
	RegisterPieceAction(EPieceAction::TakePieceDamage, Payload);

	CurrentHealth -= (int32)DamageAmount;
	if (CurrentHealth <= 0)
	{
		RegisterPieceAction(EPieceAction::Die);
		APieceActor* InstigatorPiece = Cast<APieceActor>(DamageCauser);
		BoardManager->OnPieceDeath(this, InstigatorPiece);
	}

	return DamageAmount;
}

void APieceActor::OnPieceDestroyed(APieceActor* PieceInstigator)
{
}

FPieceAction APieceActor::GetPieceAction(const FTurnAction& GameAction)
{
	int32 PieceID;
	int32 ActionID;
	TArray<uint8> Payload;
	Payload.SetNum(GameAction.Payload.Num() - 2 * sizeof(int32));

	FMemory::Memcpy(&PieceID, GameAction.Payload.GetData(), sizeof(int32));
	FMemory::Memcpy(&ActionID, &GameAction.Payload[4], sizeof(int32));

	// Retrieve payload only if there is one available
	if(Payload.Num() != 0)
		FMemory::Memcpy(Payload.GetData(), &GameAction.Payload[8], Payload.Num());

	return FPieceAction(PieceID, ActionID, Payload);
}

void APieceActor::OnDeployPieceInSquareAction(int32 SquareIndex)
{
	FVector DeployLocation = BoardManager->GetSquareLocation(SquareIndex);
	SetActorLocation(DeployLocation);

	BoardManager->OnGameActionComplete();
}

/* --------------- Process turn -------------------- */ 

void APieceActor::ProcessTurn()
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
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveForward);
	}
}

void APieceActor::Process_MoveToSquare(const int32& TargetSquareIndex, int32 ActionID)
{
	TArray<uint8> Payload;
	Payload.SetNum(sizeof(int32));
	FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));

	BoardManager->MovePieceToSquare(this, TargetSquareIndex);

	ABoardSquare* BoardSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
	if (BoardSquare)
		BoardSquare->Process_SetSquarePlayerColor(DavidPlayerOwner);

	RegisterPieceAction(ActionID, Payload);
}

void APieceActor::Process_AttackPieceInSquare(const int32& TargetSquareIndex, int32 ActionID)
{
	APieceActor* PieceToAttack = BoardManager->GetPieceInSquare(TargetSquareIndex);

	if (PieceToAttack && PieceToAttack->GetOwnerPlayer() != DavidPlayerOwner)
	{
		TArray<uint8> Payload;
		Payload.SetNum(sizeof(int32));
		FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));
		RegisterPieceAction(ActionID, Payload);

		UGameplayStatics::ApplyDamage(PieceToAttack, CurrentAttack, nullptr, this, nullptr);
	}
}

/* -------------- Play actions ------------------------ */

void APieceActor::ProcessAction(const FPieceAction& Action)
{
	switch (Action.ActionID)
	{
		case EPieceAction::MoveForward: 
		{
			Action_MoveToSquare(Action.Payload);
			break;
		}
		case EPieceAction::FrontAttack: 
		{
			Action_AttackFrontPiece();
			break;
		}
		case EPieceAction::TakePieceDamage:
		{
			Action_TakeDamage(Action.Payload);
			break;
		}
		case EPieceAction::Die: 
		{
			Action_Die();
			break;
		}
		default: 
		{
			BoardManager->OnGameActionComplete();
			break;
		}
	}
}

void APieceActor::Action_MoveToSquare(const TArray<uint8>& Payload)
{
	// Get the target square from the payload
	int32 TargetSquareIndex;
	FMemory::Memcpy(&TargetSquareIndex, Payload.GetData(), sizeof(int32));

	bIsMoving = true;
	MovementDelta = 0.f;
	OriginLocation = GetActorLocation();

	TargetLocation = BoardManager->GetSquareLocation(TargetSquareIndex);

	TargetSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
}

void APieceActor::Action_AttackFrontPiece()
{
	BoardManager->OnGameActionComplete();
}

void APieceActor::Action_TakeDamage(const TArray<uint8>& Payload)
{
	ABoardManager* BM = BoardManager;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([BM]
	{
		UE_LOG(LogDavid, Display, TEXT("Ouch!"));
		BM->OnGameActionComplete();
	});

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 0.5f, false);
}

void APieceActor::Action_Die()
{
	APieceActor* Piece = this;
	ABoardManager* BM = BoardManager;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([BM, Piece]
	{
		BM->RemoveActivePiece(Piece);
		Piece->Destroy();
		BM->OnGameActionComplete();
	});

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 1.f, false);
}

void APieceActor::HandlePieceMovement(float DeltaSeconds)
{
	// Add the delta time to the counter and calculate the movement status
	MovementDelta += DeltaSeconds;
	const float MovementStatus = FMath::Clamp(MovementDelta / MovementTime, 0.f, 1.f);

	// Finished movement
	if (MovementStatus >= 1.f) 
	{
		bIsMoving = false;
		BoardManager->OnGameActionComplete();
		SetActorLocation(TargetLocation);

		if (TargetSquare)
			TargetSquare->Action_SetSquarePlayerColor(DavidPlayerOwner);

		return;
	}

	// Calculate this frame movement position and apply to actor
	const FVector MovementPosition = FMath::Lerp(OriginLocation, TargetLocation, MovementStatus);
	SetActorLocation(MovementPosition);
}
