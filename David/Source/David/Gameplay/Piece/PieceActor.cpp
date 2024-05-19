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

	BoardManager->MovePieceToSquare(this, TargetSquareIndex);

	ABoardSquare* BoardSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
	if (BoardSquare)
		BoardSquare->Process_SetSquarePlayerColor(DavidPlayerOwner);

	RegisterPieceAction(ActionID, Payload);
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

	MovementTargetSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
}

void APieceActor::Action_AttackFrontPiece()
{
	BoardManager->OnGameActionComplete();
}

void APieceActor::Action_TakeDamage(const TArray<uint8>& Payload)
{
	// Get actor health from the action
	int32 IncomingHealth;
	FMemory::Memcpy(&IncomingHealth, Payload.GetData(), sizeof(int32));

	UE_LOG(LogDavid, Display, TEXT("Attack received. %d health left"), IncomingHealth);

	// Set current health
	CurrentHealth = IncomingHealth;

	// Change the displayed health in the stats widget
	if(StatsWidget)
		StatsWidget->SetHealthValue(IncomingHealth);

	BoardManager->OnGameActionComplete();
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

		if (MovementTargetSquare)
			MovementTargetSquare->Action_SetSquarePlayerColor(DavidPlayerOwner);

		return;
	}

	if (!PieceMovementCurve) return;

	// Evaluate movement curve
	FVector MovementEvaluation = PieceMovementCurve->GetVectorValue(MovementStatus);

	// Calculate this frame movement position and apply to actor
	FVector MovementPosition = (TargetLocation - OriginLocation) * MovementEvaluation.Y;
	MovementPosition.X += MovementEvaluation.X;
	MovementPosition.Z += MovementEvaluation.Z;

	SetActorLocation(OriginLocation + MovementPosition);
}
