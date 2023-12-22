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
	void Tick(float DeltaSeconds) override;

	bool GetBoardHitUnderCursor(FHitResult& Result, const FVector2D MousePosition);

	UFUNCTION(Client, Reliable)
	void Client_SetDavidPlayerIndex(int32 PIndex);

protected:

	virtual void BeginPlay() override;

	void HandleInput();

	void ProcessGeneralInteraction();

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

private:
	UPROPERTY(SkipSerialization, Transient)
	UUserWidget* PlayerHUD;

	int32 PlayerIndex;
};
