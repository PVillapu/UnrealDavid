#pragma once

#include "CoreMinimal.h"
#include "CardData.h"
#include "GameCardData.generated.h"

USTRUCT()
struct FGameCardData
{
    GENERATED_USTRUCT_BODY()

    FGameCardData() { }

    FGameCardData(const FCardData& CardData)
    {
        PieceHealth = CardData.PieceHealth;
        PieceAttack = CardData.PieceAttack;
        bIsPieceCard = CardData.bIsPiece;
        CardCost = CardData.CardCost;
    }

    UPROPERTY()
    int32 CardDTIndex = -1;

    UPROPERTY()
    int32 GameCardID = -1;

    UPROPERTY()
    int32 CardCost = 0;

    UPROPERTY()
    bool bIsPieceCard = true;

    // Piece data

    UPROPERTY()
    int32 PieceHealth = 0;

    UPROPERTY()
    int32 PieceAttack = 0;
};
