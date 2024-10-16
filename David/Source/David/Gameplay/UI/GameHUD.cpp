#include "GameHUD.h"
#include "../DavidGameState.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "../Player/DavidPlayerController.h"
#include "../DavidPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "../Piece/PieceActor.h"
#include "../Cards/CardData.h"
#include "CardWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "HandManager.h"
#include "../DavidGameInstance.h"

void UGameHUD::OnCursorOverPiece(APieceActor* PieceSelected)
{
	CurrentInspectedPiece = PieceSelected;

	PlaceInfoCardInViewport();
}

void UGameHUD::OnCursorLeftPiece()
{
	CurrentInspectedPiece = nullptr;
	PieceInfoCard->SetVisibility(ESlateVisibility::Hidden);
}

void UGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Start the Game and Player state checker
	World->GetTimerManager().SetTimer(PlayerStateCheckTimerHandler, this, &UGameHUD::CheckForAvailablePlayerState, 0.2f, true);

	EndgameOverlay->SetVisibility(ESlateVisibility::Hidden);
}

void UGameHUD::NativeDestruct()
{
	Super::NativeDestruct();

	if (DavidGameState == nullptr) return;

	// Unbind delegates
	DavidGameState->OnPlayerTurnChangedDelegate.Remove(OnPlayerTurnChangedDelegateHandler);
	DavidGameState->OnPlayerTurnTimeUpdatedDelegate.Remove(OnPlayerTurnTimeUpdatedDelegateHandler);
	DavidGameState->OnPlayersScoreChangesDelegate.Remove(OnPlayersScoreChangedDelegateHandler);
	DavidGameState->OnRoundCompletedDelegate.Remove(OnRoundCompletedDelegateHandler);
	DavidGameState->OnGameFinishedDelegate.Remove(OnGameFinishedDelegateHandler);
	EndTurnButton->OnClicked.RemoveDynamic(this, &UGameHUD::OnPlayerPressedEndTurnButton);
}

void UGameHUD::SetupGameHUD(ADavidGameState* GameState, ADavidPlayerState* PlayerState)
{
	DavidGameState = Cast<ADavidGameState>(GameState);
	if (DavidGameState == nullptr) return;
	
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	World->GameStateSetEvent.Remove(OnGameStateReplicatedDelegateHandler);

	EndTurnButton->SetIsEnabled(false);
	EndTurnButton->SetRenderOpacity(0.5f);

	// Bind delegates
	OnPlayerTurnChangedDelegateHandler = DavidGameState->OnPlayerTurnChangedDelegate.AddUObject(this, &UGameHUD::OnMatchStateChanged);
	OnPlayerTurnTimeUpdatedDelegateHandler = DavidGameState->OnPlayerTurnTimeUpdatedDelegate.AddUObject(this, &UGameHUD::OnPlayerTurnTimeUpdated);
	OnPlayersScoreChangedDelegateHandler = DavidGameState->OnPlayersScoreChangesDelegate.AddUObject(this, &UGameHUD::OnPlayersScoreChanges);
	OnRoundCompletedDelegateHandler = DavidGameState->OnRoundCompletedDelegate.AddUObject(this, &UGameHUD::OnRoundCompleted);
	OnGameFinishedDelegateHandler = DavidGameState->OnGameFinishedDelegate.AddUObject(this, &UGameHUD::OnGameFinished);
	EndTurnButton->OnClicked.AddDynamic(this, &UGameHUD::OnPlayerPressedEndTurnButton);

	// Bind PlayerGold updates
	if(PlayerState)
		PlayerState->OnPlayerGoldChanged.AddUObject(this, &UGameHUD::OnPlayerGoldUpdates);

	// Init player hand manager
	HandManagerWidget->InitializeHandManager();

	// Mark HUD initialization as done
	if(ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController()))
		DavidPlayerController->InitializationPartDone(EDavidPreMatchInitialization::PLAYER_HUD_INITIALIZED);

	// Hide the information card
	PieceInfoCard->SetVisibility(ESlateVisibility::Hidden);

	// Place the info card at 0,0 in the UI
	UCanvasPanelSlot* CardPanelSlot = Cast<UCanvasPanelSlot>(PieceInfoCard->Slot);
	if(CardPanelSlot)
		CardPanelSlot->SetPosition(FVector2D::ZeroVector);
}

void UGameHUD::OnMatchStateChanged(EDavidMatchState PlayerTurn)
{
	if (PlayerTurn == EDavidMatchState::END_GAME) return;

	if (ADavidPlayerController* DavidPlayerController = GetOwningPlayer<ADavidPlayerController>())
	{
		if (DavidPlayerController->IsPlayerTurn())
		{
			EndTurnButton->SetIsEnabled(true);
			EndTurnButton->SetRenderOpacity(1.f);

			CurrentPlayerTurnText->SetText(FText::FromString(FString::Printf(TEXT("Your turn"))));

			return;
		}
	}
	else return;

	// Disable the button if its not the player turn
	EndTurnButton->SetIsEnabled(false);
	EndTurnButton->SetRenderOpacity(0.5f);

	CurrentPlayerTurnText->SetText(FText::FromString(FString::Printf(TEXT("Opponent turn"))));
}

void UGameHUD::OnPlayerTurnTimeUpdated(int32 TurnTimeRemaining)
{
	TurnTimeRemainingText->SetText(FText::FromString(FString::Printf(TEXT("%d"), TurnTimeRemaining)));
}

void UGameHUD::OnPlayerPressedEndTurnButton()
{
	if (DavidGameState) 
	{
		ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(GetWorld()->GetFirstPlayerController());
		
		if (DavidPlayerController) 
		{
			DavidPlayerController->Server_EndTurnButtonPressed();
			EndTurnButton->SetIsEnabled(false);
			EndTurnButton->SetRenderOpacity(0.5f);
		}
	}
}

void UGameHUD::OnPlayerGoldUpdates(int32 PlayerGold)
{
	PlayerGoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), PlayerGold)));
}

void UGameHUD::OnPlayersScoreChanges(int32 Player1Score, int32 Player2Score)
{
	PlayersScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d vs %d"), Player1Score, Player2Score)));
}

void UGameHUD::OnRoundCompleted(int32 TotalRounds)
{
	CurrentRoundText->SetText(FText::FromString(FString::Printf(TEXT("Round %d"), TotalRounds)));
}

void UGameHUD::OnGameFinished(int32 Winner)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ADavidPlayerController* PlayerController = World->GetFirstPlayerController<ADavidPlayerController>();
	EDavidPlayer LocalDavidPlayer = PlayerController->GetDavidPlayer();

	if (Winner == 0)	// Tie
	{
		EndgameText->SetText(FText::FromString(FString::Printf(TEXT("Tie"))));
	}
	else if ((LocalDavidPlayer == EDavidPlayer::PLAYER_1 && Winner == 1) || (LocalDavidPlayer == EDavidPlayer::PLAYER_2 && Winner == 2))	// Local player won
	{
		EndgameText->SetText(FText::FromString(FString::Printf(TEXT("You win!"))));
	}
	else    // Local player lost
	{
		EndgameText->SetText(FText::FromString(FString::Printf(TEXT("You lost!"))));
	}

	EndgameOverlay->SetVisibility(ESlateVisibility::Visible);
}

void UGameHUD::CheckForAvailablePlayerState()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if(DavidGameState == nullptr)
		DavidGameState = Cast<ADavidGameState>(World->GetGameState());

	if (DavidGameState) 
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		for (int i = 0; i < DavidGameState->PlayerArray.Num(); ++i)
		{
			// Check if the player state object is available
			if (DavidGameState->PlayerArray[i]->GetPlayerController() == PlayerController) 
			{
				// Stop timer
				World->GetTimerManager().ClearTimer(PlayerStateCheckTimerHandler);
				
				// Get DavidPlayerState reference to call the setup function
				ADavidPlayerState* DavidPlayerState = Cast<ADavidPlayerState>(DavidGameState->PlayerArray[i]);
				SetupGameHUD(DavidGameState, DavidPlayerState);
				return;
			}
		}
	}
}

void UGameHUD::PlaceInfoCardInViewport()
{
	int32 CardDataIndex = CurrentInspectedPiece->GetCardDataIndex();

	UDavidGameInstance* GameInstance = Cast<UDavidGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if(GameInstance == nullptr) return;

	UDataTable* CardsDataTable = GameInstance->GetPieceCardsDataTable();
	if (CardsDataTable == nullptr) return;

	const TArray<FCardData>& CardsArray = GameInstance->GetGameCards();
	if (CardDataIndex < 0 || CardDataIndex >= CardsArray.Num()) return;

	const FCardData& CardData = CardsArray[CardDataIndex];

	PieceInfoCard->SetupInfoCard(CardData, CurrentInspectedPiece);
	PieceInfoCard->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UWorld* World = GetWorld())
	{
		FVector2D ViewportPosition = FVector2D::ZeroVector;

		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) 
		{
			// Get actor location
			FVector ActorLocation = CurrentInspectedPiece->GetActorLocation();

			// Project actor location to screen space
			PlayerController->ProjectWorldLocationToScreen(ActorLocation, ViewportPosition);
		}

		if (UCanvasPanelSlot* CardCanvasSlot = Cast<UCanvasPanelSlot>(PieceInfoCard->Slot))
		{
			//ViewportPosition += FVector2D(CardCanvasSlot->GetSize().X, -CardCanvasSlot->GetSize().Y * 0.5f);
			FWidgetTransform TargetTransform(ViewportPosition, PieceInfoCard->GetRenderTransform().Scale, PieceInfoCard->GetRenderTransform().Shear, 0.f);
			PieceInfoCard->SetRenderTransform(TargetTransform);
		}
	}
}
