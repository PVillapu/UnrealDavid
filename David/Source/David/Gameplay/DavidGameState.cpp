#include "DavidGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Board/BoardManager.h"
#include "Player/DavidPlayerController.h"
#include "EngineUtils.h"
#include "DavidPlayerState.h"
#include "Player/PlayerCards.h"
#include "Misc/CustomDavidLogs.h"

ADavidGameState::ADavidGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ADavidGameState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Get BoardManager reference
	if (BoardManager == nullptr)
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
		if (OutActors.Num() > 0)
		{
			BoardManager = Cast<ABoardManager>(OutActors[0]);
		}
	}
}

void ADavidGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADavidGameState, MatchState);
	DOREPLIFETIME(ADavidGameState, CurrentTurnTimeLeft);
}

void ADavidGameState::StartTurnsCycle()
{
	// Choose the start turn player
	int32 RandNum = FMath::RandRange(0, 1);
	MatchState = RandNum == 0 ? EDavidMatchState::PLAYER_1_TURN : EDavidMatchState::PLAYER_2_TURN;
	StartPlayerTurn(MatchState == EDavidMatchState::PLAYER_1_TURN ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2);
	OnMatchStateChange();

	// Start the turn time timer
	CurrentTurnTimeLeft = PlayerTurnTime;
	GetWorld()->GetTimerManager().SetTimer(TurnTimeLeftTimerHandler, this, &ADavidGameState::UpdateTurnCountdownTime, 1.0f, true);

	// Each player draws initial cards
	ADavidPlayerController* PlayerController = GetPlayerController(EDavidPlayer::PLAYER_1);
	if (PlayerController)
	{
		APlayerCards* PlayerCards = PlayerController->GetPlayerCards();
		PlayerCards->PlayerDrawCards(InitialCardsDrawAmmount);
	}

	PlayerController = GetPlayerController(EDavidPlayer::PLAYER_2);
	if (PlayerController)
	{
		APlayerCards* PlayerCards = PlayerController->GetPlayerCards();
		PlayerCards->PlayerDrawCards(InitialCardsDrawAmmount);
	}
}

void ADavidGameState::OnPlayerFinishedTurn(EDavidPlayer Player)
{
	// Check that the request is made by the valid player
	if (Player == EDavidPlayer::PLAYER_1 && MatchState != EDavidMatchState::PLAYER_1_TURN
		|| Player == EDavidPlayer::PLAYER_2 && MatchState != EDavidMatchState::PLAYER_2_TURN) 
		return;

	ChangeMatchState();
}

void ADavidGameState::UpdateTurnCountdownTime()
{
	if (--CurrentTurnTimeLeft <= 0) 
	{
		GetWorld()->GetTimerManager().PauseTimer(TurnTimeLeftTimerHandler);
		ChangeMatchState();
	}

	OnTurnTimeUpdated();
}

void ADavidGameState::ChangeMatchState()
{
	if (MatchState == EDavidMatchState::PLAYER_1_TURN || MatchState == EDavidMatchState::PLAYER_2_TURN) 
	{
		LastTurnPlayer = MatchState == EDavidMatchState::PLAYER_1_TURN ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2;
		MatchState = EDavidMatchState::PROCESSING_TURN;
		ProcessPlayerTurn(LastTurnPlayer == EDavidMatchState::PLAYER_1_TURN ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2);
	}
	else if (MatchState == EDavidMatchState::PROCESSING_TURN) 
	{
		MatchState = LastTurnPlayer == EDavidPlayer::PLAYER_1 ? EDavidMatchState::PLAYER_2_TURN : EDavidMatchState::PLAYER_1_TURN;
		StartPlayerTurn(LastTurnPlayer == EDavidPlayer::PLAYER_1 ? EDavidPlayer::PLAYER_2 : EDavidPlayer::PLAYER_1);

		// Set round time
		CurrentTurnTimeLeft = PlayerTurnTime;
		GetWorld()->GetTimerManager().UnPauseTimer(TurnTimeLeftTimerHandler);
		OnTurnTimeUpdated();
	}

	OnMatchStateChange();
}

void ADavidGameState::OnRep_MatchState()
{
	OnMatchStateChange();
}

void ADavidGameState::OnRep_CurrentTurnTimeLeft()
{
	OnTurnTimeUpdated();
}

void ADavidGameState::OnMatchStateChange() const
{
	UE_LOG(LogDavid, Display, TEXT("[%s] ADavidGameState::OnMatchStateChange()"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));

	OnPlayerTurnChangedDelegate.Broadcast(MatchState.GetValue());
}

void ADavidGameState::OnTurnTimeUpdated() const
{
	OnPlayerTurnTimeUpdatedDelegate.Broadcast(CurrentTurnTimeLeft);
}

void ADavidGameState::ProcessPlayerTurn(EDavidPlayer Player)
{
	if (BoardManager == nullptr) return;

	BoardManager->ProcessPlayerTurn(Player);
	BoardManager->PlayTurnActions();

	ChangeMatchState();
}

void ADavidGameState::StartPlayerTurn(EDavidPlayer Player) 
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Search the PlayerState to add the initial turn gold
	for (ADavidPlayerController* DavidPlayerController : TActorRange<ADavidPlayerController>(World))
	{
		if (DavidPlayerController->GetDavidPlayer() == Player) 
		{
			ADavidPlayerState* PlayerGameState = DavidPlayerController->GetPlayerState<ADavidPlayerState>();
			PlayerGameState->IncreasePlayerGold(BaseGoldEarnedAtTurnBegin);
			return;
		}
	}
}

ADavidPlayerController* ADavidGameState::GetPlayerController(EDavidPlayer Player)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	// Search for the player controller with the given ID
	for (int i = 0; i < World->GetNumPlayerControllers(); ++i) 
	{
		ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(UGameplayStatics::GetPlayerController(World, i));
		if (PlayerController && PlayerController->GetDavidPlayer() == Player) return PlayerController;
	}
	
	// Not found
	return nullptr;
}
