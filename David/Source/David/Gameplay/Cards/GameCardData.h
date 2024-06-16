#pragma once

#include "CoreMinimal.h"
#include "GameCardData.generated.h"

USTRUCT()
struct FGameCardData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int32 CardDTIndex = -1;

    UPROPERTY()
    int32 CardID = -1;

    UPROPERTY()
    int32 PieceHealth = 0;

    UPROPERTY()
    int32 PieceAttack = 0;
};
