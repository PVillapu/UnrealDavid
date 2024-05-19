#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 * Class that handles the player HUD
 */
UCLASS()
class DAVID_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:

	void SetupHUD(class APlayerController& PlayerController);

	FORCEINLINE class UGameHUD* GetGameHUDWidget() const { return GameHUDWidget; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	TSubclassOf<UUserWidget> GameplayWidgetClass;

	UPROPERTY(Transient, SkipSerialization)
	class ADavidGameState* DavidGameState;

	UPROPERTY(Transient, SkipSerialization)
	UGameHUD* GameHUDWidget;
};
