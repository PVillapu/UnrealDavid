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
	void OnCursorOverPiece(class APieceActor* PieceSelected);

	void OnCursorLeftPiece();

	FORCEINLINE class UHandManager* GetPlayerHandManager() { return HandManagerWidget; }

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

	UFUNCTION()
	void OnRoundCompleted(int32 TotalRounds);

	UFUNCTION()
	void OnGameFinished(int32 Winner);

	void CheckForAvailablePlayerState();

	void PlaceInfoCardInViewport();

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UCanvasPanel* CanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UHandManager* HandManagerWidget;

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

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* CurrentRoundText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UOverlay* EndgameOverlay;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* EndgameText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UCardWidget* PieceInfoCard;

	UPROPERTY(EditAnywhere, Category = "David")
	int32 PieceInfoCardHorizontalOffset = 100;

	UPROPERTY(Transient, SkipSerialization)
	ADavidGameState* DavidGameState;

private:
	UPROPERTY(Transient, SkipSerialization)
	APieceActor* CurrentInspectedPiece;

	// Delegate handlers
	FDelegateHandle OnPlayerTurnChangedDelegateHandler;

	FDelegateHandle OnPlayerTurnTimeUpdatedDelegateHandler;

	FDelegateHandle OnGameStateReplicatedDelegateHandler; // TODO: Check

	FDelegateHandle OnPlayersScoreChangedDelegateHandler;

	FDelegateHandle OnRoundCompletedDelegateHandler;

	FDelegateHandle OnGameFinishedDelegateHandler;

	// Timer handler
	FTimerHandle PlayerStateCheckTimerHandler;
};
