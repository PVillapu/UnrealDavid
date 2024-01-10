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

	// Performs a line trace over the board and returns the result ------------ REMOVE ----------
	bool GetBoardHitUnderCursor(FHitResult& Result, const FVector2D& MousePosition);

	/* Sets the player index.Should only be called by the server */ 
	FORCEINLINE void SetPlayerIndex(EDavidPlayer Index) { PlayerIndex = Index; SetupPlayer(); }

	/* Returns the Player player*/
	FORCEINLINE EDavidPlayer GetDavidPlayer() { return PlayerIndex; }

	/* Sets the PlayerCards actor of this player controller */
	FORCEINLINE void SetPlayerCards(class APlayerCards* PlayerCardsActor) { PlayerCards = PlayerCardsActor;	}

	/* Returns this player APlayerCards */
	FORCEINLINE APlayerCards* GetPlayerCards() { return PlayerCards; }

	/* Returns the cards datatable */
	FORCEINLINE class UDataTable* GetCardsDataTable() { return CardsDataTable; }

	/* Returns the player game HUD */
	FORCEINLINE class UGameHUD* GetPlayerGameHUD() { return PlayerHUD; }

	/* Returns if its this player controller turn */
	bool IsPlayerTurn();

	/* Called when a mandatory pre match part of the game has been initialized */
	void InitializationPartDone(EDavidPreMatchInitialization Part);

	UFUNCTION(Server, reliable)
	void Server_EndTurnButtonPressed();

	/* Called by the client when this PlayerController Client is ready to start */
	UFUNCTION(Server, reliable)
	void Server_PlayerReady();

	UFUNCTION(Server, reliable)
	void Server_PlayerActionsProcessed();

private:
	UFUNCTION()
	void OnRep_PlayerIndex();

	void SetupPlayer();

	void CreatePlayerHUD();

private:
	UPROPERTY(EditDefaultsOnly, Category = "David")
	UDataTable* CardsDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	TEnumAsByte<ECollisionChannel> ActionsTraceChannel = ECC_Pawn;

	UPROPERTY(EditDefaultsOnly, Category = "David")
	TEnumAsByte<ECollisionChannel> BoardCollisionChannel;

	UPROPERTY(SkipSerialization, Transient)
	AActor* PlayerCameraActor;

	UPROPERTY(SkipSerialization, Transient)
	class UCameraComponent* PlayerCamera;

	UPROPERTY(SkipSerialization, Transient)
	class UGameHUD* PlayerHUD;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerIndex)
	TEnumAsByte<EDavidPlayer> PlayerIndex;

	UPROPERTY(SkipSerialization, Transient)
	APlayerCards* PlayerCards;

	UPROPERTY(SkipSerialization, Transient)
	TArray<bool> PreMatchInitializedParts;
};
