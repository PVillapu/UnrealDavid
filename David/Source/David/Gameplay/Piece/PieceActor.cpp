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
#include "UObject/ConstructorHelpers.h"

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
	SkeletalMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 20.f));

	StatsWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatsWidget"));
	StatsWidgetComponent->SetupAttachment(SkeletalMeshComponent);

	SkeletalMeshComponent->OnBeginCursorOver.AddDynamic(this, &APieceActor::OnBeginCursorOverEvent);
	SkeletalMeshComponent->OnEndCursorOver.AddDynamic(this, &APieceActor::OnEndCursorOverEvent);

	// Default BP references

	// static ConstructorHelpers::FObjectFinder<UClass> DefaultMovementCurveBP(TEXT("Class'/Game/Blueprints/Gameplay/Pieces/Curves/Movement/C_BasePieceMovement.C_BasePieceMovement_C'"));
	// if(DefaultMovementCurveBP.Succeeded())
	// {
	// 	PieceMovementCurve = (UCurveVector*)DefaultMovementCurveBP.Object;
	// }
	
	// static ConstructorHelpers::FObjectFinder<UClass> DefaultAttackCurveBP(TEXT("/Game/Blueprints/Gameplay/Pieces/Curves/Attack/C_BasePieceAttack'"));
	// if(DefaultAttackCurveBP.Succeeded())
	// {
	// 	PieceAttackCurve = (UCurveVector*)DefaultAttackCurveBP.Object;
	// }

	// static ConstructorHelpers::FObjectFinder<UClass> DefaultReceiveDamagePositionCurveBP(TEXT("/Game/Blueprints/Gameplay/Pieces/Curves/ReceiveDamage/C_BasePieceReceiveDamageLocationOffset"));
	// if(DefaultReceiveDamagePositionCurveBP.Succeeded())
	// {
	// 	PieceReceiveDamageCurvePosition = (UCurveVector*)DefaultReceiveDamagePositionCurveBP.Object;
	// }

	// static ConstructorHelpers::FObjectFinder<UClass> DefaultReceiveDamageRotationCurveBP(TEXT("/Game/Blueprints/Gameplay/Pieces/Curves/ReceiveDamage/C_BasePieceReceiveDamageRotation"));
	// if(DefaultReceiveDamageRotationCurveBP.Succeeded())
	// {
	// 	PieceReceiveDamageCurveRotation = (UCurveVector*)DefaultReceiveDamageRotationCurveBP.Object;
	// }
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

void APieceActor::Process_TakeDamage(int32 DamageAmount)
{
	ProcessHealth -= DamageAmount;

	if (ProcessHealth <= 0)
	{
		// Remove the piece from board
		BoardManager->Process_RemovePieceFromProcessBoard(this);
	}
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

void APieceActor::Process_HoldDamage(AActor *DamageCauser)
{
	// Register action
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
}

void APieceActor::OnPieceDestroyed(APieceActor* PieceInstigator)
{
	SkeletalMeshComponent->OnBeginCursorOver.RemoveAll(this);
}

void APieceActor::OnPieceReachedEndLine()
{
	BoardManager->Process_RemovePieceFromProcessBoard(this);
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

void APieceActor::LogPieceEvent(const FString& Message) const
{
	UE_LOG(LogDavidGameEvent, Log, TEXT("[Piece event] Piece: %s, BoardIndex: %d | %s"), *GetName(), Square->GetSquareIndex(), *Message);
}

void APieceActor::OnDeployPieceInSquareAction(int32 SquareIndex)
{
	SetActorLocation(BoardManager->GetSquarePieceLocation(SquareIndex));
	const FRotator FacingDirection = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? FVector::ForwardVector.ToOrientationRotator() : FVector::BackwardVector.ToOrientationRotator();
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

	const APieceActor* TargetPiece = BoardManager->GetPieceInSquare(TargetSquareIndex);
	if (BoardManager->IsSquareOccupied(TargetSquareIndex) && !IsPieceOfSameTeam(TargetPiece)) // Attack
	{
		Process_AttackPiece(TargetSquareIndex);
	}
	else if(!BoardManager->IsSquareOccupied(TargetSquareIndex)) // Move forward
	{
		Process_MoveToSquare(TargetSquareIndex, EPieceAction::MoveToSquare);
	}
}

void APieceActor::Process_MoveToSquare(int32 TargetSquareIndex, int32 ActionID)
{
	TArray<uint8> Payload;
	Payload.SetNum(sizeof(int32));
	FMemory::Memcpy(Payload.GetData(), &TargetSquareIndex, sizeof(int32));

	UE_LOG(LogDavid, Warning, TEXT("Process_MoveToSquare : Target: %d"), TargetSquareIndex);
	RegisterPieceAction(ActionID, Payload);

	BoardManager->MovePieceToSquare(this, TargetSquareIndex);

}

void APieceActor::Process_AttackPiece(int32 TargetSquareIndex)
{
	APieceActor* PieceToAttack = BoardManager->GetPieceInSquare(TargetSquareIndex);

	if (PieceToAttack)
	{
		Process_AttackPiece(PieceToAttack);
	}
}

void APieceActor::Process_AttackPiece(APieceActor *TargetPiece)
{
	UE_LOG(LogDavid, Warning, TEXT("Process_AttackPiece : Target: %d"), TargetPiece->GetBoardSquare()->GetSquareIndex());
	RegisterPieceAction(EPieceAction::FrontAttack);

	BoardManager->Process_AttackPiece(TargetPiece, this, ProcessAttack);
}

void APieceActor::Process_AttackPieces(const TArray<int32>& TargetSquareIndex)
{
	TArray<APieceActor*> PiecesToAttack;

	UE_LOG(LogDavid, Warning, TEXT("Process_AttackPieces : TargetCount: %d"), TargetSquareIndex.Num());

	// Gather pieces to attack
	for(int i = 0; i < TargetSquareIndex.Num(); ++i)
	{
		APieceActor* Piece = BoardManager->GetPieceInSquare(TargetSquareIndex[i]);
		if(Piece)
		{
			PiecesToAttack.Add(Piece);
		}
	}

	if (PiecesToAttack.Num() > 0)
	{
		RegisterPieceAction(EPieceAction::FrontAttack);
		BoardManager->Process_AttackMultiplePieces(PiecesToAttack, this, ProcessAttack);
	}
}

void APieceActor::Process_AttackPieces(TArray<APieceActor*>& TargetPieces)
{
	if (TargetPieces.Num() > 0)
	{
		RegisterPieceAction(EPieceAction::FrontAttack);
		BoardManager->Process_AttackMultiplePieces(TargetPieces, this, ProcessAttack);
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
		#if WITH_EDITOR

			if (GEngine)
			{
				FString Message = FString::Printf(TEXT("Action_MoveToSquare | From %d to %d"), Square->GetSquareIndex(), TargetSquareIndex);
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(Message)
				);
			}

		#endif

		bIsMoving = true;
		MovementDelta = 0.f;
		Action_OriginLocation = GetActorLocation();

		Action_TargetLocation = BoardManager->GetSquarePieceLocation(TargetSquareIndex);
		Action_TargetSquare = BoardManager->GetBoardSquare(TargetSquareIndex);
		Square = Action_TargetSquare;
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
		#if WITH_EDITOR

			if (GEngine)
			{
				FString Message = FString::Printf(TEXT("Action_AttackFrontPiece"));
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(Message)
				);
			}

		#endif

		bIsAttacking = true;
		AttackDelta = 0.f;

		Action_OriginLocation = GetActorLocation();
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

		Action_OriginLocation = GetActorLocation();

		// Get actor health from the action
		int32 IncomingHealth;
		FMemory::Memcpy(&IncomingHealth, Payload.GetData(), sizeof(int32));

		#if WITH_EDITOR

			if (GEngine)
			{
				FString Message = FString::Printf(TEXT("Action_TakeDamage | Left health: %d"), IncomingHealth);
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(Message)
				);
			}

		#endif

		// Set current health
		CurrentHealth = IncomingHealth;

		// Change the displayed health in the stats widget
		if (StatsWidget)
			StatsWidget->SetHealthValue(CurrentHealth);
	}
	else
	{
		BoardManager->OnGameActionComplete();
	}
}

void APieceActor::Action_Die()
{
	#if WITH_EDITOR

			if (GEngine)
			{
				FString Message = FString::Printf(TEXT("Action_Die"));
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(Message)
				);
			}

	#endif

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
	#if WITH_EDITOR

			if (GEngine)
			{
				FString Message = FString::Printf(TEXT("Action_ReachedEndLine"));
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Blue,
					FString(Message)
				);
			}

	#endif

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
		SetActorLocation(Action_TargetLocation);
		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	FVector MovementEvaluation = PieceMovementCurve->GetVectorValue(MovementStatus);

	// Calculate this frame movement position and apply to actor
	FVector MovementPosition = (Action_TargetLocation - Action_OriginLocation) * MovementEvaluation.Y;
	MovementPosition.X += MovementEvaluation.X;
	MovementPosition.Z += MovementEvaluation.Z;

	SetActorLocation(Action_OriginLocation + MovementPosition);
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
		SetActorLocation(Action_OriginLocation);
		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	const FVector AttackEvaluation = PieceMovementCurve->GetVectorValue(AttackStatus);

	// Calculate this frame movement position and apply to actor
	SetActorLocation(Action_OriginLocation + AttackEvaluation);
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

		SetActorLocation(Action_OriginLocation);
		const FRotator FacingDirection = DavidPlayerOwner == EDavidPlayer::PLAYER_1 ? FVector::ForwardVector.ToOrientationRotator() : FVector::BackwardVector.ToOrientationRotator();
		SetActorRotation(FacingDirection);

		BoardManager->OnGameActionComplete();

		return;
	}

	// Evaluate movement curve
	const FVector ActionPositionEvaluation = PieceReceiveDamageCurvePosition->GetVectorValue(ActionStatus);

	const FVector PieceRotation = PieceReceiveDamageCurveRotation->GetVectorValue(ActionStatus);
	FRotator Rot;
	Rot.Pitch = PieceRotation.X;
	Rot.Yaw = PieceRotation.Y;
	Rot.Roll = PieceRotation.Z;

	// Calculate this frame movement position and apply to actor
	const FVector SquarePosition = Action_OriginLocation;

	SetActorLocation(SquarePosition + ActionPositionEvaluation);
	SetActorRotation(Rot);
}
