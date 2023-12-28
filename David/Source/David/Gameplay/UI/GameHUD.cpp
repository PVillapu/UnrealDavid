#include "GameHUD.h"
#include "../DavidGameState.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "../DavidGameState.h"

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
	OnPlayerTurnChangedDelegateHandler = DavidGameState->OnPlayerTurnChangedDelegate.AddUObject(this, &UGameHUD::OnPlayerTurnChanged);
	OnPlayerTurnTimeUpdatedDelegateHandler = DavidGameState->OnPlayerTurnTimeUpdatedDelegate.AddUObject(this, &UGameHUD::OnPlayerTurnTimeUpdated);
	EndTurnButton->OnClicked.AddDynamic(this, &UGameHUD::OnPlayerPressedEndTurnButton);
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

void UGameHUD::OnPlayerTurnChanged(EDavidPlayer PlayerTurn)
{
	FString PlayerTurnString = UEnum::GetValueAsString(PlayerTurn);

	EndTurnButton->SetIsEnabled(true);
	EndTurnButton->SetRenderOpacity(1.f);

	UE_LOG(LogBlueprint, VeryVerbose, TEXT("Player turn changed to %s"), *PlayerTurnString);
}

void UGameHUD::OnPlayerTurnTimeUpdated(int32 TurnTimeRemaining)
{
	TurnTimeRemainingText->Text = FText::FromString(FString::Printf(TEXT("%d"), TurnTimeRemaining));
}

void UGameHUD::OnPlayerPressedEndTurnButton()
{
	if (DavidGameState) 
	{
		DavidGameState->Server_EndTurnButtonPressed();
		EndTurnButton->SetIsEnabled(false);
		EndTurnButton->SetRenderOpacity(0.5f);
	}
}
