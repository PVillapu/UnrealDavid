#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameRules.generated.h"

/**
 * Holds the information of a game set of rules
 */
UCLASS(BlueprintType)
class DAVID_API UDavidGameRules : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player gold")
    int32 MaxGold = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player gold")
    int32 StartTurnGoldEarned = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player hand")
    int32 MaxPlayerHandSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player hand")
    int32 CardDrawAmmountPerTurn = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player hand")
    int32 InitialCardsDrawAmmount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player turns")
    float TurnDuration = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match config")
    int32 GameTotalRounds = 20;
};