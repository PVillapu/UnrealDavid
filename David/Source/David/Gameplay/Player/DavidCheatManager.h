#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "DavidCheatManager.generated.h"

/**
 * Adds custom cheats for development or testing purposes.
 */
UCLASS()
class DAVID_API UDavidCheatManager : public UCheatManager
{
	GENERATED_BODY()

	class ADavidPlayerController* DavidPC;

	virtual void InitCheatManager();

	UFUNCTION(exec)
	virtual void InfiniteGold();

	UFUNCTION(exec)
	virtual void InfiniteTurnDuration();

	UFUNCTION(exec)
	virtual void GiveCardToPlayer(int32 CardId, int32 Player);
};