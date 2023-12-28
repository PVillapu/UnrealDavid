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

	DOREPLIFETIME(ADavidGameState, CurrentPlayerTurn);
	DOREPLIFETIME(ADavidGameState, CurrentTurnTimeLeft);
}

void ADavidGameState::StartTurnsCycle()
{
	ChangePlayerTurn();
	GetWorld()->GetTimerManager().SetTimer(TurnCountdownTimerHandler, this, &ADavidGameState::UpdateTurnCountdownTime, 1.0f, true);
}

void ADavidGameState::Server_EndTurnButtonPressed_Implementation()
{
	// This is where you can access information about the client that called the RPC.
	UNetConnection* NetConnection = GetNetConnection();
	if (NetConnection)
	{
		FString ClientName = NetConnection->PlayerController ? NetConnection->PlayerController->GetName() : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("RPC called by client: %s"), *ClientName);
	}
}

void ADavidGameState::OnRep_CurrentPlayerTurn()
{
	UE_LOG(LogTemp, Log, TEXT("Turn changed!"));

	OnPlayerTurnChangedDelegate.Broadcast((EDavidPlayer)CurrentPlayerTurn);
}

void ADavidGameState::OnRep_CurrentTurnTimeLeft()
{
	UE_LOG(LogTemp, Log, TEXT("Seconds left: %d"), CurrentTurnTimeLeft);

	OnPlayerTurnTimeUpdatedDelegate.Broadcast(CurrentTurnTimeLeft);
}

void ADavidGameState::UpdateTurnCountdownTime()
{
	if (--CurrentTurnTimeLeft <= 0) 
	{
		GetWorld()->GetTimerManager().PauseTimer(TurnCountdownTimerHandler);
		ChangePlayerTurn();
	}
}

void ADavidGameState::ChangePlayerTurn()
{
	CurrentPlayerTurn = CurrentPlayerTurn == (uint8)EDavidPlayer::PLAYER_1 ? (uint8)EDavidPlayer::PLAYER_2 : (uint8)EDavidPlayer::PLAYER_1;
	CurrentTurnTimeLeft = PlayerRoundTime;
	GetWorld()->GetTimerManager().UnPauseTimer(TurnCountdownTimerHandler);
}
