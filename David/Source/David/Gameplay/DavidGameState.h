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
	/* Class constructor */
	ADavidGameState();

	void BeginPlay() override;

	/* Replication setup */ 
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Starts the gameplay server flow */ 
	void StartTurnsCycle();

	/* Called when a player ends its turn */ 
	void OnPlayerFinishedTurn(EDavidPlayer Player);

	void OnPlayerPlayedTurnActions();

	/* Called during action play. Called by clients */
	void Action_IncreasePlayerScore(EDavidPlayer Player, int32 ScoreAmmount);

	/* Called by the client when all actions has been played */
	void OnTurnActionsProcessed();

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_SetFinalTurnScore(int32 Player1Score, int32 Player2Score);

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_GameEnded(int32 Winner);

	UFUNCTION(NetMulticast, reliable)
	void NetMulticast_CurrentRoundUpdated(int32 Round);
	
	FORCEINLINE EDavidMatchState GetMatchState() const { return MatchState; }

private:
	/* Called on Server to update the turns time left */
	void UpdateTurnCountdownTime();

	/* Changes the match state to the following one */ 
	void ChangeMatchState();

	/* Calculates the current player scores and send them to clients */
	void SendEndTurnScores();

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION()
	void OnRep_CurrentTurnTimeLeft();

	/* Called when MatchState changes */
	void OnMatchStateChange() const;

	/* Called when TurnTime is updated */
	void OnTurnTimeUpdated() const;

	/* Called when a player turn ends and turn must be processed */
	void ProcessPlayerEndTurn(EDavidPlayer Player);

	/* Called when a player turn starts */
	void StartPlayerTurn(EDavidPlayer Player);

	/* Returns the player controller of the current player turn */
	class ADavidPlayerController* GetPlayerController(EDavidPlayer Player);

	void OnGameFinished();

public:
	/* Called when the player turn changes */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnChanged, EDavidMatchState)
	FOnPlayerTurnChanged OnPlayerTurnChangedDelegate;

	/* Called when the turn time left is updated */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnTimeUpdated, int32)
	FOnPlayerTurnTimeUpdated OnPlayerTurnTimeUpdatedDelegate;

	/* Called when player score changes */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayersScoreChanges, int32, int32)
	FOnPlayersScoreChanges OnPlayersScoreChangesDelegate;

	/* Called when a round is completed */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoundCompleted, int32)
	FOnRoundCompleted OnRoundCompletedDelegate;

	/* Called when a round is completed */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameFinished, int32)
	FOnGameFinished OnGameFinishedDelegate;

	/* Stores the match state */
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	TEnumAsByte<EDavidMatchState> MatchState;

	/* Stores the time left of the current turn */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnTimeLeft)
	int32 CurrentTurnTimeLeft;

private:
	/* Player turn time */
	UPROPERTY(EditDefaultsOnly, Category = "David")
	int32 PlayerTurnTime = 30;

	/* Total rounds until endgame */
	UPROPERTY(EditDefaultsOnly, Category = "David")
	int32 GameTotalRounds = 20;

	/* Ammount of gold earned by a player at the start of the turn */
	UPROPERTY(EditDefaultsOnly, Category = "David")
	int32 BaseGoldEarnedAtTurnBegin = 2;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	int32 InitialCardsDrawAmmount = 3;

	/* Timer handle for turn time */
	UPROPERTY(Transient, SkipSerialization)
	FTimerHandle TurnTimeLeftTimerHandler;

	UPROPERTY(SkipSerialization, Transient)
	class ABoardManager* BoardManager;

	UPROPERTY(Transient, SkipSerialization)
	TEnumAsByte<EDavidPlayer> LastTurnPlayer;

	/* Current rounds played in game */
	UPROPERTY(Transient, SkipSerialization)
	int32 RoundsPlayed;

	/* Auxiliar counter for rounds control */
	UPROPERTY(Transient, SkipSerialization)
	int32 RoundTurnsPlayed;

	/* Server-driven scores */
	UPROPERTY(Transient, SkipSerialization)
	int32 ScorePlayer1;
	UPROPERTY(Transient, SkipSerialization)
	int32 ScorePlayer2;

	/* Client-driven scores */
	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentPlayer1Score;
	UPROPERTY(Transient, SkipSerialization)
	int32 CurrentPlayer2Score;

	int32 ClientActionsProcessed = 0;
};