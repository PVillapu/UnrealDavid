#pragma once

#include "CoreMinimal.h"
#include "CardData.h"
#include "GameCardData.generated.h"

USTRUCT()
struct FGameCardData
{
    GENERATED_USTRUCT_BODY()

    FGameCardData() { }

    FGameCardData(FCardData& CardData)
    {
        PieceHealth = CardData.PieceHealth;
        PieceAttack = CardData.PieceAttack;
        CardCost = CardData.CardCost;
    }

    UPROPERTY()
    int32 CardDTIndex;

    UPROPERTY()
    int32 CardID;

    UPROPERTY()
    int32 PieceHealth;

    UPROPERTY()
    int32 PieceAttack;

    UPROPERTY()
    int32 CardCost;
};
