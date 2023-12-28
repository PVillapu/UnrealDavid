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
	ADavidGameState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void StartTurnsCycle();

	UFUNCTION(Server, reliable)
	void Server_EndTurnButtonPressed();

protected:
	UFUNCTION()
	void OnRep_CurrentPlayerTurn();

	UFUNCTION()
	void OnRep_CurrentTurnTimeLeft();

private:
	void UpdateTurnCountdownTime();

	void ChangePlayerTurn();

public:
	// Called when the player turn changes
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnChanged, EDavidPlayer)
	FOnPlayerTurnChanged OnPlayerTurnChangedDelegate;

	// Called when the turn time left is updated
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnTimeUpdated, int32)
	FOnPlayerTurnTimeUpdated OnPlayerTurnTimeUpdatedDelegate;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerTurn)
	uint8 CurrentPlayerTurn;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnTimeLeft)
	int32 CurrentTurnTimeLeft;

protected:
	UPROPERTY(EditAnywhere, Category = "David")
	int32 PlayerRoundTime = 30;

private:
	FTimerHandle TurnCountdownTimerHandler;
};