#include "DavidPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "../Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "../DavidGameState.h"
#include "../UI/GameHUD.h"
#include "../Player/PlayerCards.h"
#include "../UI/HandManager.h"
#include "Engine/DataTable.h"
#include "../Cards/CardData.h"

void ADavidPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController()) return;

	// Create gameplay HUD and attach to viewport
	PlayerHUD = CreateWidget<UGameHUD>(GetGameInstance(), PlayerHUDClass);
	if (PlayerHUD != nullptr)
	{
		PlayerHUD->AddToViewport();
		PlayerHUD->GetPlayerHandManager()->SetPlayerCards(PlayerCards);
	}

	// Set input mode to only UI
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void ADavidPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADavidPlayerController, PlayerIndex);
}

bool ADavidPlayerController::IsPlayerTurn()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return false;

	ADavidGameState* DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return false;

	return (DavidGameState->GetMatchState() == EDavidMatchState::PLAYER_1_TURN && PlayerIndex == EDavidPlayer::PLAYER_1)
		|| (DavidGameState->GetMatchState() == EDavidMatchState::PLAYER_2_TURN && PlayerIndex == EDavidPlayer::PLAYER_2);
}

bool ADavidPlayerController::GetBoardHitUnderCursor(FHitResult& Hit, const FVector2D& MousePosition)
{
	FIntPoint MousePos;
	GEngine->GameViewport->Viewport->GetMousePos(MousePos);
	FVector2D MousePosV = FVector2D(MousePos.X, MousePos.Y);

	UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *MousePosV.ToString());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector WorldLocation, WorldDirection;
	UGameplayStatics::DeprojectScreenToWorld(this, MousePosV, WorldLocation, WorldDirection);
	
	FVector CameraLocation = PlayerCameraManager->GetCameraLocation();

	GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, (WorldLocation - CameraLocation) * 100000, BoardCollisionChannel, QueryParams);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 10.f, 0U, 1.f);

	if (Hit.bBlockingHit && IsValid(Hit.GetActor()))
	{
		UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *Hit.GetActor()->GetName());
		DrawDebugLine(GetWorld(), CameraLocation, (WorldLocation - CameraLocation) * 100000, FColor::Blue, false, 10.f, 0U, 1.f);
		return true;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("No Actors were hit"));
		return false;
	}
}

void ADavidPlayerController::Server_EndTurnButtonPressed_Implementation()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidGameState* DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return;

	// Call gamestate in order to finish the turn
	DavidGameState->OnPlayerFinishedTurn(PlayerIndex);
}

void ADavidPlayerController::Server_RequestPlayCard_Implementation(FName CardRowName, int32 SquareID, int32 PlayID)
{

}

void ADavidPlayerController::OnRep_PlayerIndex()
{
	SetupPlayer();
}

void ADavidPlayerController::SetupPlayer()
{
	if (!IsLocalController()) return;

	// Get reference to board manager if is not already set
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Get BoardManager reference
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
	if (OutActors.Num() > 0)
	{
		ABoardManager* BoardManager = Cast<ABoardManager>(OutActors[0]);

		// Get the player camera spot
		PlayerCameraActor = BoardManager->GetPlayerCameraActor(PlayerIndex);
		UActorComponent* CameraSpot = PlayerCameraActor->GetComponentByClass(UCameraComponent::StaticClass());

		if (CameraSpot == nullptr) return;

		PlayerCamera = Cast<UCameraComponent>(CameraSpot);

		// Set view target to the camera
		if (PlayerCameraActor)
		{
			SetViewTarget(PlayerCameraActor);
		}
	}

	// Spawn the player cards manager
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	FTransform SpawnTransform;

	PlayerCards = Cast<APlayerCards>(World->SpawnActor(APlayerCards::StaticClass(), &SpawnTransform, SpawnParameters));

	// WIP for the moment we just take all existing cards in the CardsDataTable
	TArray<FName> PlayerCardNames = CardsDataTable->GetRowNames();
	PlayerCards->SetupPlayerCards(PlayerCardNames);
}
