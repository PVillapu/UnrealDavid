#include "DavidPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "../Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Net/UnrealNetwork.h"
#include "../DavidGameState.h"
#include "../UI/GameHUD.h"
#include "../Player/PlayerCards.h"
#include "../UI/HandManager.h"
#include "Engine/DataTable.h"
#include "../Cards/CardData.h"
#include "../DavidGameMode.h"
#include "../Misc/CustomDavidLogs.h"
#include "../DavidPlayerState.h"

void ADavidPlayerController::SetupPlayer()
{
	if (!IsLocalPlayerController()) return;

	UE_LOG(LogDavid, Display, TEXT("[%s] ADavidPlayerController::SetupPlayer()"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));

	CreatePlayerHUD();

	// Get reference to board manager if is not already set
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Get BoardManager reference
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
	if (OutActors.Num() > 0)
	{
		ABoardManager* BoardManager = Cast<ABoardManager>(OutActors[0]);

		if (BoardManager == nullptr) return;

		BoardManager->InitializeBoard();

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

	bEnableMouseOverEvents = true;

	InitializationPartDone(EDavidPreMatchInitialization::PLAYER_INITIALIZED);
}

void ADavidPlayerController::CreatePlayerHUD()
{
	// Create gameplay HUD and attach to viewport
	PlayerHUD = CreateWidget<UGameHUD>(this, PlayerHUDClass);
	if (PlayerHUD != nullptr)
	{
		PlayerHUD->AddToViewport();
	}

	// Set input mode to game and UI
	FInputModeGameAndUI InputMode;
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

void ADavidPlayerController::InitializationPartDone(EDavidPreMatchInitialization Part)
{
	UE_LOG(LogDavid, Display, TEXT("[%s] ADavidPlayerController::InitializationPartDone : %d"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"), (int)Part);

	// Initialize the array if this is the first call
	if (PreMatchInitializedParts.Num() == 0)
	{
		PreMatchInitializedParts.Init(false, (int)EDavidPreMatchInitialization::MAX_VALUE);
	}

	// Check the initialized part
	PreMatchInitializedParts[Part] = true;

	// Check if there is any part left to initialize
	for (int i = 0; i < PreMatchInitializedParts.Num(); ++i)
		if (!PreMatchInitializedParts[i]) return;

	Server_PlayerReady();
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
		DrawDebugLine(GetWorld(), CameraLocation, (WorldLocation - CameraLocation) * 100000, FColor::Blue, false, 10.f, 0U, 1.f);
		return true;
	}
	else {
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

void ADavidPlayerController::Server_PlayerReady_Implementation()
{
	UE_LOG(LogDavid, Display, TEXT("[%s] ADavidPlayerController::Server_PlayerReady_Implementation()"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));
	if (UWorld* World = GetWorld())
	{
		if (ADavidGameMode* GameMode = Cast<ADavidGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->OnPlayerReady(PlayerIndex);
		}
	}
}

void ADavidPlayerController::Server_PlayerActionsProcessed_Implementation()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidGameState* DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return;

	// Notify gamestate
	DavidGameState->OnPlayerPlayedTurnActions();
}

void ADavidPlayerController::OnTurnActionsCompleted()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidGameState* DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return;

	DavidGameState->OnTurnActionsProcessed();

	Server_PlayerActionsProcessed();
}

void ADavidPlayerController::OnRep_PlayerIndex()
{
	SetupPlayer();
}