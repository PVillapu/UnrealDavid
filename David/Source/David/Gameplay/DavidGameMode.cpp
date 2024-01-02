#include "DavidGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Gameplay/Player/DavidPlayerController.h"
#include "DavidGameState.h"
#include "DavidPlayerState.h"

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

	// Get number of players in match
	if (GameState == nullptr) return;
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	
	// Set the player index of the joined player
	ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(NewPlayer);
	if (DavidPlayerController)
	{
		DavidPlayerController->SetPlayerIndex(NumberOfPlayers == 1 ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2);
	}

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
	
	// If both players has login, start the match
	if (NumberOfPlayers == 2) 
	{
		StartGame();
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

void ADavidGameMode::StartGame()
{
	if (GameState == nullptr) return;

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
