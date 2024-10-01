#include "DavidPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "DavidGameState.h"

void ADavidPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADavidPlayerState, PlayerGold);
}

void ADavidPlayerState::IncreasePlayerGold(int32 Ammount)
{
	CatchGameRules();

	PlayerGold += FMath::Clamp(PlayerGold + Ammount, 0, GameRules->MaxGold);
	OnPlayerGoldUpdated();
}

void ADavidPlayerState::DecreasePlayerGold(int32 Ammount)
{
	CatchGameRules();

	PlayerGold = FMath::Clamp(PlayerGold - Ammount, 0, GameRules->MaxGold);
	OnPlayerGoldUpdated();
}

void ADavidPlayerState::CatchGameRules()
{
	if(GameRules) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidGameState* DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return;

	GameRules = DavidGameState->GetGameRules();
}

void ADavidPlayerState::OnRep_PlayerGold()
{
	OnPlayerGoldUpdated();
}

void ADavidPlayerState::OnPlayerGoldUpdated()
{
	OnPlayerGoldChanged.Broadcast(PlayerGold);
}