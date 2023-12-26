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
	bool GetBoardHitUnderCursor(FHitResult& Result, const FVector2D MousePosition);

	UFUNCTION(Client, Reliable)
	void Client_SetDavidPlayerIndex(int32 PIndex);

protected:
	void SetupInputComponent() override;

	virtual void BeginPlay() override;

	void ProcessGeneralInteraction();

	void OnPlayedCardAction();

protected:
	UPROPERTY(EditAnywhere, Category = "David")
	TEnumAsByte<ECollisionChannel> ActionsTraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, Category = "David")
	TEnumAsByte<ECollisionChannel> BoardCollisionChannel;

	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY(SkipSerialization, Transient)
	AActor* PlayerCameraActor;

	UPROPERTY(SkipSerialization, Transient)
	class UCameraComponent* PlayerCamera;

	UPROPERTY(SkipSerialization, Transient)
	class ABoardManager* BoardManager;

	UPROPERTY(EditAnywhere, Category = "David")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "David")
	class UInputAction* PlayCardAction;

private:
	UPROPERTY(SkipSerialization, Transient)
	UUserWidget* PlayerHUD;

	int32 PlayerIndex;
};
