#include "DavidGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Board/BoardManager.h"
#include "Player/DavidPlayerController.h"
#include "EngineUtils.h"
#include "DavidPlayerState.h"
#include "Player/PlayerCards.h"
#include "Misc/CustomDavidLogs.h"
#include "Misc/GameRules.h"
#include "DavidGameInstance.h"

ADavidGameState::ADavidGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ADavidGameState::BeginPlay()
{
	Super::BeginPlay();

	CurrentPlayer1Score = CurrentPlayer2Score = 0;

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
	RoundsPlayed = 0;
	RoundTurnsPlayed = 0;

	// Choose the start turn player
	int32 RandNum = FMath::RandRange(0, 1);
	MatchState = RandNum == 0 ? EDavidMatchState::PLAYER_1_TURN : EDavidMatchState::PLAYER_2_TURN;
	StartPlayerTurn(MatchState == EDavidMatchState::PLAYER_1_TURN ? EDavidPlayer::PLAYER_1 : EDavidPlayer::PLAYER_2);
	OnMatchStateChange();

	// Start the turn time timer
	CurrentTurnTimeLeft = GameRules->TurnDuration;
	GetWorld()->GetTimerManager().SetTimer(TurnTimeLeftTimerHandler, this, &ADavidGameState::UpdateTurnCountdownTime, 1.0f, true);

	ScorePlayer1 = ScorePlayer2 = 0;

	// Each player draws initial cards
	ADavidPlayerController* PlayerController = GetPlayerController(EDavidPlayer::PLAYER_1);
	if (PlayerController)
	{
		APlayerCards* PlayerCards = PlayerController->GetPlayerCards();
		PlayerCards->PlayerDrawCards(GameRules->InitialCardsDrawAmmount);
	}

	PlayerController = GetPlayerController(EDavidPlayer::PLAYER_2);
	if (PlayerController)
	{
		APlayerCards* PlayerCards = PlayerController->GetPlayerCards();
		PlayerCards->PlayerDrawCards(GameRules->InitialCardsDrawAmmount);
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

void ADavidGameState::OnPlayerPlayedTurnActions()
{
	if (MatchState != EDavidMatchState::PROCESSING_TURN) return;

	if (++ClientActionsProcessed == 2) ChangeMatchState();
}

void ADavidGameState::SendEndTurnScores()
{
	// Calculate players score
	BoardManager->CalculatePlayersScore(ScorePlayer1, ScorePlayer2);

	// Send to clients
	NetMulticast_SetFinalTurnScore(ScorePlayer1, ScorePlayer2);
}

void ADavidGameState::Action_IncreasePlayerScore(EDavidPlayer Player, int32 ScoreAmmount)
{
	if (Player == EDavidPlayer::PLAYER_1)
	{
		CurrentPlayer1Score += ScoreAmmount;
	}
	else
	{
		CurrentPlayer2Score += ScoreAmmount;
	}

	OnPlayersScoreChangesDelegate.Broadcast(CurrentPlayer1Score, CurrentPlayer2Score);
}

void ADavidGameState::OnTurnActionsProcessed()
{
	// Check if current client score is the same as the server end round score
	if (ScorePlayer1 != CurrentPlayer1Score || ScorePlayer2 != CurrentPlayer2Score) 
	{
		CurrentPlayer1Score = ScorePlayer1;
		CurrentPlayer2Score = ScorePlayer2;
		OnPlayersScoreChangesDelegate.Broadcast(CurrentPlayer1Score, CurrentPlayer2Score);
	}
}

void ADavidGameState::NetMulticast_GameEnded_Implementation(int32 Winner)
{
	OnGameFinishedDelegate.Broadcast((EDavidPlayer)Winner);
}

void ADavidGameState::NetMulticast_SetFinalTurnScore_Implementation(int32 Score1, int32 Score2)
{
	ScorePlayer1 = Score1;
	ScorePlayer2 = Score2;
}

void ADavidGameState::NetMulticast_CurrentRoundUpdated_Implementation(int32 Round)
{
	OnRoundCompletedDelegate.Broadcast(Round);
}

void ADavidGameState::UpdateTurnCountdownTime()
{
#if UE_WITH_CHEAT_MANAGER
	if(bInfiniteTurnDurationCheat) return;
#endif
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
		OnMatchStateChange();
		ProcessPlayerEndTurn(LastTurnPlayer);
	}
	else if (MatchState == EDavidMatchState::PROCESSING_TURN) 
	{
		// Check if round is over
		if (++RoundTurnsPlayed == 2) 
		{	
			RoundsPlayed++;
			NetMulticast_CurrentRoundUpdated(RoundsPlayed);

			// Endgame
			if (RoundsPlayed >= GameRules->GameTotalRounds) 
			{
				OnGameFinished();
				return;
			}

			RoundTurnsPlayed = 0;
		}

		MatchState = LastTurnPlayer == EDavidPlayer::PLAYER_1 ? EDavidMatchState::PLAYER_2_TURN : EDavidMatchState::PLAYER_1_TURN;
		OnMatchStateChange();
		StartPlayerTurn(LastTurnPlayer == EDavidPlayer::PLAYER_1 ? EDavidPlayer::PLAYER_2 : EDavidPlayer::PLAYER_1);

		// Set round time
		CurrentTurnTimeLeft = GameRules->TurnDuration;
		GetWorld()->GetTimerManager().UnPauseTimer(TurnTimeLeftTimerHandler);
		OnTurnTimeUpdated();
	}
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
	UE_LOG(LogDavid, Display, TEXT("[%s] ADavidGameState::OnMatchStateChange to %d"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"), (int)MatchState.GetValue());

	OnPlayerTurnChangedDelegate.Broadcast(MatchState.GetValue());
}

void ADavidGameState::OnTurnTimeUpdated() const
{
	OnPlayerTurnTimeUpdatedDelegate.Broadcast(CurrentTurnTimeLeft);
}

void ADavidGameState::ProcessPlayerEndTurn(EDavidPlayer Player)
{
	// Reset the client actions processed count
	ClientActionsProcessed = 0;

	// Process the player turn
	BoardManager->ProcessPlayerEndTurn(Player);
	
	// Calculate players end round score and send to clients
	SendEndTurnScores();
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
			// Increase the player gold
			ADavidPlayerState* PlayerState = DavidPlayerController->GetPlayerState<ADavidPlayerState>();
			PlayerState->IncreasePlayerGold(GameRules->StartTurnGoldEarned);
			
			// Draw 1 card
			APlayerCards* PlayerCards = DavidPlayerController->GetPlayerCards();
			if (PlayerCards) 
			{
				PlayerCards->PlayerDrawCards(1);
			}
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

TArray<struct FCardData>* ADavidGameState::GetGameCards()
{
	UDavidGameInstance* GameInstance = Cast<UDavidGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if(GameInstance == nullptr) return nullptr;
	
    return &GameInstance->GetGameCards();
}

void ADavidGameState::OnGameFinished()
{
	MatchState = EDavidMatchState::END_GAME;

	// Winner: 0 -> Tie / 1 -> P1 / 2 -> P2
	int32 Winner;
	if (ScorePlayer1 > ScorePlayer2) Winner = 1;
	else if (ScorePlayer1 < ScorePlayer2) Winner = 2;
	else Winner = 0;

	// Stop turn timer
	GetWorld()->GetTimerManager().ClearTimer(TurnTimeLeftTimerHandler);

	NetMulticast_GameEnded(Winner);
}
