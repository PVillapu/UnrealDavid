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

	void SetupInputComponent() override;

	void Tick(float DeltaSeconds) override;

protected:

	void HandleInput();

	void ProcessGeneralInteraction();

	void PlayCard();

public:

	UPROPERTY(EditAnywhere, Category = "Game Player")
	class UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = "Game Player")
	TEnumAsByte<ECollisionChannel> ActionsTraceChannel = ECC_Pawn;
};
