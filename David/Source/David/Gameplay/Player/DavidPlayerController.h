#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DavidPlayerController.generated.h"

/**
 * Class that handles the player input & actions in the game
 */
UCLASS()
class DAVID_API ADavidPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADavidPlayerController();

	void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	void HandleInput();

	void ProcessGeneralInteraction();

	//void PlayCardAction(const class FInputActionValue& Value);

protected:

	UPROPERTY(EditAnywhere, Category = "Game Player")
	class UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = "Game Player")
	TEnumAsByte<ECollisionChannel> ActionsTraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, Category = "Game Player")
	TSubclassOf<UUserWidget> PlayerHUDClass;

private:
	UPROPERTY()
	UUserWidget* PlayerHUD;
};
