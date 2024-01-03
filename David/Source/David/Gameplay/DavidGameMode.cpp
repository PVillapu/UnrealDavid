#include "DavidGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Gameplay/Player/DavidPlayerController.h"
#include "DavidGameState.h"
#include "DavidPlayerState.h"
#include "Engine/DataTable.h"
#include "Player/PlayerCards.h"
#include "Misc/CustomDavidLogs.h"

ADavidGameMode::ADavidGameMode()
{
	// Set default game classes
	PlayerControllerClass = ADavidPlayerController::StaticClass();
	GameStateClass = ADavidGameState::StaticClass();
	PlayerStateClass = ADavidPlayerState::StaticClass();
}

void ADavidGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
	if(OutActors.Num() > 0)
	{
		BoardManager = Cast<ABoardManager>(OutActors[0]);
	}
}

void ADavidGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogDavid, Display, TEXT("[Server] ADavidGameMode::PostLogin()"));

	// Get number of players in match
	if (GameState == nullptr) return;
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	
	// Set the player index of the joined player
	ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(NewPlayer);
	if (DavidPlayerController)
	{
		DavidPlayerController->SetPlayerIndex(NumberOfPlayers == 1 ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2);
	}

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Spawn the player cards manager
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = NewPlayer;
	FTransform SpawnTransform;
	APlayerCards* PlayerCards = Cast<APlayerCards>(World->SpawnActor(APlayerCards::StaticClass(), &SpawnTransform, SpawnParameters));
	DavidPlayerController->SetPlayerCards(PlayerCards);

	// WIP for the moment we just take all existing cards in the CardsDataTable
	TArray<FName> PlayerCardNames = DavidPlayerController->GetCardsDataTable()->GetRowNames();
	PlayerCards->SetupPlayerCards(PlayerCardNames);

	// Debug message
	if (GEngine)
	{
		APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if (NewPlayerState)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("%s joined the game! %d players in game"), *NewPlayerState->GetName(), NumberOfPlayers)
			);
		}
	}
}

void ADavidGameMode::Logout(AController* ExitPlayer)
{
	Super::Logout(ExitPlayer);

	APlayerState* ExitPlayerState = ExitPlayer->GetPlayerState<APlayerState>();
	if (GEngine && ExitPlayerState)
	{
		GEngine->AddOnScreenDebugMessage(
			2,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("%s has left the game"), *ExitPlayerState->GetName())
		);
	}
}

void ADavidGameMode::OnPlayerReady(EDavidPlayer Player)
{
	if (MatchStarted) return;

	if (Player == EDavidPlayer::PLAYER_1) Player1Ready = true;
	else Player2Ready = true;

	if (Player2Ready && Player1Ready)
	{
		MatchStarted = true;
		StartGame();
	}
}

void ADavidGameMode::StartGame()
{
	ADavidGameState* DavidGameState = Cast<ADavidGameState>(GameState);

	if (DavidGameState == nullptr) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			3,
			15.f,
			FColor::White,
			FString::Printf(TEXT("Game starts"))
		);
	}
	DavidGameState->StartTurnsCycle();
}
