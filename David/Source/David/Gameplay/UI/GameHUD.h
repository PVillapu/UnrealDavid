#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Misc/Enums.h"
#include "GameHUD.generated.h"

/**
 * Master Widget of the gameplay level
 */
UCLASS()
class DAVID_API UGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE class UHandManager* GetPlayerHandManager() { return HandManager; }

private:
	void NativeConstruct() override;

	void NativeDestruct() override;

	UFUNCTION()
	void SetupGameHUD(class ADavidGameState* GameState, class ADavidPlayerState* PlayerState);

	UFUNCTION()
	void OnMatchStateChanged(EDavidMatchState PlayerTurn);

	UFUNCTION()
	void OnPlayerTurnTimeUpdated(int32 TurnTimeRemaining);

	UFUNCTION()
	void OnPlayerPressedEndTurnButton();

	UFUNCTION()
	void OnPlayerGoldUpdates(int32 PlayerGold);

	UFUNCTION()
	void OnPlayersScoreChanges(int32 Player1Score, int32 Player2Score);

	void CheckForAvailablePlayerState();

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UCanvasPanel* CanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UHandManager* HandManager;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* TurnTimeRemainingText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UButton* EndTurnButton;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* CurrentPlayerTurnText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PlayerGoldText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PlayersScoreText;

	UPROPERTY(Transient, SkipSerialization)
	ADavidGameState* DavidGameState;

private:
	// Delegate handlers
	FDelegateHandle OnPlayerTurnChangedDelegateHandler;

	FDelegateHandle OnPlayerTurnTimeUpdatedDelegateHandler;

	FDelegateHandle OnGameStateReplicatedDelegateHandler; // TODO: Check

	FDelegateHandle OnPlayersScoreChangedDelegateHandler;

	// Timer handler
	FTimerHandle PlayerStateCheckTimerHandler;
};
