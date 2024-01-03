#include "GameHUD.h"
#include "../DavidGameState.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "../DavidGameState.h"
#include "../Player/DavidPlayerController.h"
#include "../DavidPlayerState.h"

void UGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	EndTurnButton->SetIsEnabled(false);
	EndTurnButton->SetRenderOpacity(0.5f);

	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	DavidGameState = Cast<ADavidGameState>(World->GetGameState());
	if (DavidGameState == nullptr) return;

	// Bind delegates
	OnPlayerTurnChangedDelegateHandler = DavidGameState->OnPlayerTurnChangedDelegate.AddUObject(this, &UGameHUD::OnMatchStateChanged);
	OnPlayerTurnTimeUpdatedDelegateHandler = DavidGameState->OnPlayerTurnTimeUpdatedDelegate.AddUObject(this, &UGameHUD::OnPlayerTurnTimeUpdated);
	EndTurnButton->OnClicked.AddDynamic(this, &UGameHUD::OnPlayerPressedEndTurnButton);

	if (ADavidPlayerState* DavidPlayerState = World->GetFirstPlayerController()->GetPlayerState<ADavidPlayerState>())
	{
		DavidPlayerState->OnPlayerGoldChanged.AddUObject(this, &UGameHUD::OnPlayerGoldUpdates);
	}
}

void UGameHUD::NativeDestruct()
{
	Super::NativeDestruct();

	if (DavidGameState == nullptr) return;

	// Unbind delegates
	DavidGameState->OnPlayerTurnChangedDelegate.Remove(OnPlayerTurnChangedDelegateHandler);
	DavidGameState->OnPlayerTurnTimeUpdatedDelegate.Remove(OnPlayerTurnTimeUpdatedDelegateHandler);
	EndTurnButton->OnClicked.RemoveDynamic(this, &UGameHUD::OnPlayerPressedEndTurnButton);
}

void UGameHUD::OnMatchStateChanged(EDavidMatchState PlayerTurn)
{
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
