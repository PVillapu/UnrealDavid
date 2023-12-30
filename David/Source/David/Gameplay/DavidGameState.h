#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Misc/Enums.h"
#include "DavidGameState.generated.h"

/**
 * Class that handles the Turns management and players gold
 */
UCLASS()
class DAVID_API ADavidGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	// Class constructor
	ADavidGameState();

	// Replication setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Starts the gameplay server flow
	void StartTurnsCycle();

	// Called when a player ends its turn
	void OnPlayerFinishedTurn(EDavidPlayer Player);

	FORCEINLINE EDavidMatchState GetMatchState() const { return MatchState; }

private:
	// Called on Server to update the turns time left 
	void UpdateTurnCountdownTime();

	// Changes the match state to the following one
	void ChangeMatchState();

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION()
	void OnRep_CurrentTurnTimeLeft();

	// Called when MatchState changes
	void OnMatchStateChange() const;

	// Called when TurnTime is updated
	void OnTurnTimeUpdated() const;

public:
	// Called when the player turn changes
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnChanged, EDavidMatchState)
	FOnPlayerTurnChanged OnPlayerTurnChangedDelegate;

	// Called when the turn time left is updated
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnTimeUpdated, int32)
	FOnPlayerTurnTimeUpdated OnPlayerTurnTimeUpdatedDelegate;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	TEnumAsByte<EDavidMatchState> MatchState;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnTimeLeft)
	int32 CurrentTurnTimeLeft;

protected:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 PlayerTurnTime = 30;

private:
	FTimerHandle TurnTimeLeftTimerHandler;
};