#pragma once

#include "CoreMinimal.h"
#include "PieceCardData.h"
#include "SpellCardData.h"
#include "CardData.generated.h"

USTRUCT()
struct FCardData
{
    GENERATED_USTRUCT_BODY()

    FCardData() {}

    FCardData(const FPieceCardData& PieceData)
    {
        bIsPiece = true;
        CardName = PieceData.CardName;
        CardImage = PieceData.CardImage;
        CardDescription = PieceData.CardDescription;
        CardCost = PieceData.CardCost;
        PieceAttack = PieceData.PieceAttack;
        PieceHealth = PieceData.PieceHealth;
        CardPieceClass = PieceData.CardPieceClass;
    }

    FCardData(const FSpellCardData& SpellData)
    {
        bIsPiece = false;
        CardName = SpellData.CardName;
        CardImage = SpellData.CardImage;
        CardDescription = SpellData.CardDescription;
        CardCost = SpellData.CardCost;
        CardSpellClass = SpellData.CardSpellClass;
    }

    FText CardName;
    UTexture2D* CardImage;
    FText CardDescription;
    int32 CardCost;
    bool bIsPiece;

    // Piece data
    int32 PieceAttack;
    int32 PieceHealth;
    TSubclassOf<class APieceActor> CardPieceClass;

    // Spell data 
    TSubclassOf<class UDavidSpell> CardSpellClass;
};
