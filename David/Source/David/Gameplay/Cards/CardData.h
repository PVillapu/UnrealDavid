#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CardData.generated.h"

USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    FText CardName = FText::FromString("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    TSubclassOf<class APieceActor> CardPieceActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    UTexture2D* CardImage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    FText CardDescription = FText::FromString("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    int32 PieceHealth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    int32 PieceAttack = 0;
};
