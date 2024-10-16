#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Misc/GameRules.h"
#include "DavidPlayerState.generated.h"

/**
 * Class that handles all replicated player specific data
 */
UCLASS()
class DAVID_API ADavidPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	// Replication setup
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void IncreasePlayerGold(int32 Ammount);

	void DecreasePlayerGold(int32 Ammount);

	void CatchGameRules();

	FORCEINLINE int32 GetPlayerGold() { return PlayerGold; }

private:
	UFUNCTION()
	void OnRep_PlayerGold();

	void OnPlayerGoldUpdated();

public:
	// Called when the player gold changes
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerGoldChanged, int32)
	FOnPlayerGoldChanged OnPlayerGoldChanged;

private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerGold)
	int32 PlayerGold;

	UPROPERTY(Transient, SkipSerialization)
	UDavidGameRules* GameRules;
};
