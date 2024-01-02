#pragma once

#include "CoreMinimal.h"
#include "GameCardData.generated.h"

USTRUCT()
struct FGameCardData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FName CardName;

    UPROPERTY()
    int32 CardID;

    UPROPERTY()
    int32 PieceHealth;

    UPROPERTY()
    int32 PieceAttack;
};
