#include "DavidGameState.h"
#include "Net/UnrealNetwork.h"

ADavidGameState::ADavidGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
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
	// Change match state 
	MatchState = MatchState == EDavidMatchState::PLAYER_1_TURN ? EDavidMatchState::PLAYER_2_TURN : EDavidMatchState::PLAYER_1_TURN;
	OnMatchStateChange();

	// Set round time
	CurrentTurnTimeLeft = PlayerTurnTime;
	GetWorld()->GetTimerManager().UnPauseTimer(TurnTimeLeftTimerHandler);
	OnTurnTimeUpdated();
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