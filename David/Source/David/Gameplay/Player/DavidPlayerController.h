#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Misc/Enums.h"
#include "DavidPlayerController.generated.h"

/**
 * Class that handles the player input & actions in the game
 */
UCLASS()
class DAVID_API ADavidPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Replication setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Performs a line trace over the board and returns the result
	bool GetBoardHitUnderCursor(FHitResult& Result, const FVector2D& MousePosition);

	// Sets the player index. Should only be called by the server
	FORCEINLINE void SetPlayerIndex(EDavidPlayer Index) { PlayerIndex = Index; SetupPlayer(); }

	FORCEINLINE EDavidPlayer GetPlayerIndex() { return PlayerIndex; }

	bool IsPlayerTurn();

	UFUNCTION(Server, reliable)
	void Server_EndTurnButtonPressed();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_PlayerIndex();

	void SetupPlayer();

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

	UPROPERTY(ReplicatedUsing=OnRep_PlayerIndex)
	TEnumAsByte<EDavidPlayer> PlayerIndex;
};
