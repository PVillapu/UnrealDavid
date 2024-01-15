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

void APieceActor::RegisterPieceAction(EPieceAction PieceAction) const
{
	FPieceAction Action(PieceID, PieceAction);

	BoardManager->RegisterTurnAction(Action);
}

void APieceActor::RegisterPieceAction(EPieceAction PieceAction, TArray<uint8>& Payload) const
{
	FPieceAction Action(PieceID, PieceAction, Payload);

	BoardManager->RegisterTurnAction(Action);
}

void APieceActor::DeployInSquare(int32 SquareIndex)
{
	Multicast_DeployPieceInSquare(SquareIndex);
}

float APieceActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TArray<uint8> Payload;
	Payload.Reserve(sizeof(float));
	FMemory::Memcpy(Payload.GetData(), &DamageAmount, sizeof(float));
	RegisterPieceAction(EPieceAction::TakePieceDamage, Payload);

	if (CurrentHealth -= DamageAmount <= 0)
	{
		BoardManager->OnPieceDeathInTurnProcess(this);
		RegisterPieceAction(EPieceAction::Die);
	}

	return DamageAmount;
}

void APieceActor::Multicast_DeployPieceInSquare_Implementation(int32 SquareIndex)
{
	OnDeployPieceInSquare(SquareIndex);
}

void APieceActor::OnDeployPieceInSquare(int32 SquareIndex)
{
	FVector DeployLocation = BoardManager->GetSquareLocation(SquareIndex);
	SetActorLocation(DeployLocation);
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
		APieceActor* PieceToAttack = BoardManager->GetPieceInSquare(TargetSquareIndex);
		
		if (PieceToAttack) 
		{
			TArray<uint8> Payload;
			Payload.Reserve(sizeof(int32));
			FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));
			RegisterPieceAction(EPieceAction::FrontAttack, Payload);

			UGameplayStatics::ApplyDamage(PieceToAttack, CurrentAttack, nullptr, this, nullptr);
		}
	}
	else // Move forward
	{
		TArray<uint8> Payload;
		Payload.Reserve(sizeof(int32));
		FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));

		BoardManager->MovePieceToSquare(this, TargetSquareIndex);
		
		ABoardSquare* BoardSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
		if(BoardSquare)
			BoardSquare->Process_SetSquarePlayerColor(DavidPlayerOwner);

		RegisterPieceAction(EPieceAction::MoveForward, Payload);
	}
}

void APieceActor::Process_MoveForward()
{
}

void APieceActor::Process_AttackFrontPiece()
{
}

/* -------------- Play actions ------------------------ */

void APieceActor::ProcessAction(const FPieceAction& Action)
{
	switch (Action.ActionID)
	{
		case EPieceAction::MoveForward: 
		{
			Action_MoveForward(Action.Payload);
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
			BoardManager->OnActionComplete();
			break;
		}
	}
}

void APieceActor::Action_MoveForward(const TArray<uint8>& Payload)
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
	BoardManager->OnActionComplete();

}

void APieceActor::Action_TakeDamage(const TArray<uint8>& Payload)
{
	ABoardManager* BM = BoardManager;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([BM]
	{
		UE_LOG(LogDavid, Display, TEXT("Ouch!"));
		BM->OnActionComplete();
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
		BM->OnActionComplete();
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
		BoardManager->OnActionComplete();
		SetActorLocation(TargetLocation);

		if (TargetSquare)
			TargetSquare->Action_SetSquarePlayerColor(DavidPlayerOwner);

		return;
	}

	// Calculate this frame movement position and apply to actor
	const FVector MovementPosition = FMath::Lerp(OriginLocation, TargetLocation, MovementStatus);
	SetActorLocation(MovementPosition);
}
