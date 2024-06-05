#include "PieceActor.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "../Cards/CardData.h"
#include "Kismet/GameplayStatics.h"
#include "../Board/BoardSquare.h"
#include "../Misc/CustomDavidLogs.h"
#include "../Player/DavidPlayerController.h"
#include "../UI/GameHUD.h"
#include "Components/WidgetComponent.h"
#include "../UI/PlayerHUD.h"
#include "../UI/PieceStats.h"
#include "Curves/CurveVector.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	PieceRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Piece root"));
	SetRootComponent(PieceRoot);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SkeletalMeshComponent->SetupAttachment(RootComponent);

	StatsWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatsWidget"));
	StatsWidgetComponent->SetupAttachment(SkeletalMeshComponent);

	SkeletalMeshComponent->OnBeginCursorOver.AddDynamic(this, &APieceActor::OnBeginCursorOverEvent);
	SkeletalMeshComponent->OnEndCursorOver.AddDynamic(this, &APieceActor::OnEndCursorOverEvent);
}

void APieceActor::Tick(float DeltaSeconds)
{
	if (bIsMoving) 
	{
		HandlePieceMovement(DeltaSeconds);
	}
	else if (bIsAttacking) 
	{
		HandlePieceAttack(DeltaSeconds);
	}
	else if (bIsReceivingDamage) 
	{
		HandlePieceReceiveDamage(DeltaSeconds);
	}
}

void APieceActor::SetupPiece(ABoardManager* BoardManagerActor, FGameCardData& _GameCardData, int32 ID, EDavidPlayer PieceOwner)
{
	// Setup initial data
	BoardManager = BoardManagerActor;

	CurrentHealth = BaseHealth = ProcessHealth = _GameCardData.PieceHealth;
	CurrentAttack = BaseAttack = ProcessAttack = _GameCardData.PieceAttack;

	PieceID = ID;
	DavidPlayerOwner = PieceOwner;

	GameCardData = _GameCardData;

	StatsWidget = Cast<UPieceStats>(StatsWidgetComponent->GetWidget());
	if (StatsWidget) 
	{
		StatsWidget->SetAttackValue(CurrentAttack);
		StatsWidget->SetHealthValue(CurrentHealth);
	}

	// Get game HUD reference
	if (UWorld* World = GetWorld())
	{
		if (ADavidPlayerController* DavidPlayerController = World->GetFirstPlayerController<ADavidPlayerController>()) 
		{
			if(APlayerHUD* PlayerHUD = DavidPlayerController->GetHUD<APlayerHUD>())
				GameHUD = PlayerHUD->GetGameHUDWidget();
		}
	}
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

void APieceActor::RegisterPieceAction(int32 PieceAction, const TArray<uint8>& Payload) const
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
	ProcessHealth -= (int32)DamageAmount;

	TArray<uint8> Payload;
	Payload.SetNum(sizeof(float));
	FMemory::Memcpy(Payload.GetData(), &ProcessHealth, sizeof(int32));
	RegisterPieceAction(EPieceAction::TakePieceDamage, Payload);

	if (ProcessHealth <= 0)
	{
		RegisterPieceAction(EPieceAction::Die);
		APieceActor* InstigatorPiece = Cast<APieceActor>(DamageCauser);
		BoardManager->OnPieceDeath(this, InstigatorPiece);

		InstigatorPiece->OnThisPieceDestroyedOther();
	}

	return DamageAmount;
}

void APieceActor::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	SkeletalMeshComponent->OnBeginCursorOver.RemoveAll(this);
}

void APieceActor::OnPieceReachedEndLine()
{
	BoardManager->Process_RemovePieceFromLogicBoard(this);
	RegisterPieceAction(EPieceAction::ReachedEndSquare);
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
	SetActorLocation(BoardManager->GetSquarePieceLocation(SquareIndex));
	//const FQuat FacingDirection = DavidPlayerOwner == EDavidPlayer::
	SetActorRotation(FacingDirection);

	Square = BoardManager->GetBoardSquare(SquareIndex);

	BoardManager->OnGameActionComplete();
}

FGameCardData APieceActor::GetDeathCard() const
{
	return GameCardData;
}

void APieceActor::OnBeginCursorOverEvent(UPrimitiveComponent* TouchedComponent)
{
	if(GameHUD)
	GameHUD->OnCursorOverPiece(this);
}

void APieceActor::OnEndCursorOverEvent(UPrimitiveComponent* TouchedComponent)
{
	if(GameHUD)
		GameHUD->OnCursorLeftPiece();
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
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
	}
}

void APieceActor::Process_MoveToSquare(const int32 TargetSquareIndex, const int32 ActionID)
{
	TArray<uint8> Payload;
	Payload.SetNum(sizeof(int32));
	FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));

	RegisterPieceAction(ActionID, Payload);

	BoardManager->MovePieceToSquare(this, TargetSquareIndex);
}

void APieceActor::Process_AttackPieceInSquare(const int32 TargetSquareIndex, const int32 ActionID)
{
	APieceActor* PieceToAttack = BoardManager->GetPieceInSquare(TargetSquareIndex);

	if (PieceToAttack && PieceToAttack->GetOwnerPlayer() != DavidPlayerOwner)
	{
		TArray<uint8> Payload;
		Payload.SetNum(sizeof(int32));
		FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));
		RegisterPieceAction(ActionID, Payload);

		UGameplayStatics::ApplyDamage(PieceToAttack, ProcessAttack, nullptr, this, nullptr);
	}
}

/* -------------- Play actions ------------------------ */

void APieceActor::ProcessAction(const FPieceAction& Action)
{
	switch (Action.ActionID)
	{
		case EPieceAction::MoveToSquare: 
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
		case EPieceAction::ReachedEndSquare:
		{
			Action_ReachedEndLine();
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

	if (Square && PieceMovementCurve) 
	{
		bIsMoving = true;
		MovementDelta = 0.f;
		OriginLocation = GetActorLocation();

		TargetLocation = BoardManager->GetSquarePieceLocation(TargetSquareIndex);
		MovementTargetSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
		Square = MovementTargetSquare;
	}
	else 
	{
		BoardManager->OnGameActionComplete();
	}
}

void APieceActor::Action_AttackFrontPiece()
{
	if (Square && PieceAttackCurve) 
	{
		bIsAttacking = true;
		AttackDelta = 0.f;
	}
	else
	{
		BoardManager->OnGameActionComplete();
	}
}

void APieceActor::Action_TakeDamage(const TArray<uint8>& Payload)
{
	if (Square && PieceReceiveDamageCurvePosition && PieceReceiveDamageCurveRotation)
	{
		bIsReceivingDamage = true;
		ReceiveDamageDelta = 0.f;
	}
	else
	{
		BoardManager->OnGameActionComplete();
	}
}

void APieceActor::Action_Die()
{
	APieceActor* Piece = this;
	ABoardManager* BM = BoardManager;
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([BM, Piece]
	{
		BM->RemoveActivePiece(Piece);
		BM->OnGameActionComplete();
	});

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 1.f, false);
}

void APieceActor::Action_ReachedEndLine()
{
	BoardManager->RemoveActivePiece(this);
	BoardManager->OnGameActionComplete();
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
		SetActorLocation(TargetLocation);
		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	FVector MovementEvaluation = PieceMovementCurve->GetVectorValue(MovementStatus);

	// Calculate this frame movement position and apply to actor
	FVector MovementPosition = (TargetLocation - OriginLocation) * MovementEvaluation.Y;
	MovementPosition.X += MovementEvaluation.X;
	MovementPosition.Z += MovementEvaluation.Z;

	SetActorLocation(OriginLocation + MovementPosition);
}

void APieceActor::HandlePieceAttack(float DeltaSeconds)
{
	// Add the delta time to the counter and calculate the movement status
	AttackDelta += DeltaSeconds;
	const float AttackStatus = FMath::Clamp(AttackDelta / AttackTime, 0.f, 1.f);

	// Finished movement
	if (AttackStatus >= 1.f)
	{
		bIsAttacking = false;
		SetActorLocation(Square->GetSquarePieceLocation());
		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	const FVector AttackEvaluation = PieceMovementCurve->GetVectorValue(AttackStatus);

	// Calculate this frame movement position and apply to actor
	const FVector SquarePosition = Square->GetSquarePieceLocation();

	SetActorLocation(SquarePosition + AttackEvaluation);
}

void APieceActor::HandlePieceReceiveDamage(float DeltaSeconds)
{
	// Add the delta time to the counter and calculate the movement status
	ReceiveDamageDelta += DeltaSeconds;
	const float ActionStatus = FMath::Clamp(ReceiveDamageDelta / ReceiveDamageTime, 0.f, 1.f);

	// Finished movement
	if (ActionStatus >= 1.f)
	{
		bIsReceivingDamage = false;
		SetActorLocation(Square->GetSquarePieceLocation());

		// Change the displayed health in the stats widget
		if (StatsWidget)
			StatsWidget->SetHealthValue(CurrentHealth);

		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	const FVector ActionPositionEvaluation = PieceReceiveDamageCurvePosition->GetVectorValue(ActionStatus);
	const FVector PieceRotation = PieceReceiveDamageCurveRotation->GetVectorValue(ActionStatus);

	// Calculate this frame movement position and apply to actor
	const FVector SquarePosition = Square->GetSquarePieceLocation();

	SetActorLocation(SquarePosition + ActionPositionEvaluation);
	SetActorRotation(PieceRotation.ToOrientationQuat());
}
