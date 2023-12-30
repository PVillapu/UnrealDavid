#include "DavidGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Board/BoardManager.h"
#include "Player/DavidPlayerController.h"
#include "EngineUtils.h"
#include "DavidPlayerState.h"

ADavidGameState::ADavidGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ADavidGameState::BeginPlay()
{
	if (!HasAuthority()) return;

	// Get BoardManager reference
	if (BoardManager == nullptr)
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

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
	MatchState = EDavidMatchState::PLAYER_1_TURN;
	GetWorld()->GetTimerManager().SetTimer(TurnTimeLeftTimerHandler, this, &ADavidGameState::UpdateTurnCountdownTime, 1.0f, true);
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
		ProcessPlayerTurn();
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
	OnPlayerTurnChangedDelegate.Broadcast(MatchState.GetValue());
}

void ADavidGameState::OnTurnTimeUpdated() const
{
	OnPlayerTurnTimeUpdatedDelegate.Broadcast(CurrentTurnTimeLeft);
}

void ADavidGameState::ProcessPlayerTurn()
{
	if (BoardManager == nullptr) return;


}

void ADavidGameState::StartPlayerTurn(EDavidPlayer Player) 
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Search the PlayerState to add the initial turn gold
	for (ADavidPlayerController* DavidPlayerController : TActorRange<ADavidPlayerController>(World))
	{
		if (DavidPlayerController->GetPlayerIndex() == Player) 
		{
			ADavidPlayerState* PlayerGameState = DavidPlayerController->GetPlayerState<ADavidPlayerState>();
			PlayerGameState->IncreasePlayerGold(BaseGoldEarnedAtTurnBegin);
			return;
		}
	}
}