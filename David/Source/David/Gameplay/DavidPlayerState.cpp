#include "DavidPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADavidPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADavidPlayerState, PlayerGold);
}

void ADavidPlayerState::OnRep_PlayerGold()
{
	OnPlayerGoldUpdated();
}

void ADavidPlayerState::OnPlayerGoldUpdated()
{
	OnPlayerGoldChanged.Broadcast(PlayerGold);
}