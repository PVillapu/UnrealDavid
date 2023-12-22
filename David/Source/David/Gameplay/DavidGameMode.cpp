#include "DavidGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "../Gameplay/Player/DavidPlayerController.h"

ADavidGameMode::ADavidGameMode()
{
	if (GEngine) 
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			15.f,
			FColor::Blue,
			TEXT("DavidGameMode initialized")
		);
	}
}

void ADavidGameMode::BeginPlay()
{
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

	if (GEngine && GameState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

		GEngine->AddOnScreenDebugMessage(
			1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Player joined! %d players in game"), NumberOfPlayers)
		);

		APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if (NewPlayerState)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("%s joined the game!"), *NewPlayerState->GetName())
			);
		}

		ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(NewPlayer);
		if (DavidPlayerController) 
		{
			DavidPlayerController->Client_SetDavidPlayerIndex(NumberOfPlayers - 1);
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
			1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("%s has left the game"), *ExitPlayerState->GetName())
		);
	}
}
