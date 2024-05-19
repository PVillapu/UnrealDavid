#include "PlayerHUD.h"
//#include "../DavidGameState.h"
//#include "../DavidPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GameHUD.h"

void APlayerHUD::SetupHUD(APlayerController& PlayerController)
{
	// Create Gameplay widget class
	if (GameplayWidgetClass) 
	{
		GameHUDWidget = CreateWidget<UGameHUD>(&PlayerController, GameplayWidgetClass);
		if (GameHUDWidget)
		{
			GameHUDWidget->AddToViewport();
		}
	}
}
