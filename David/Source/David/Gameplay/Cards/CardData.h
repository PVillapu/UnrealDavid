#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CardData.generated.h"

USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    TSubclassOf<class APieceActor> CardPieceActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    UTexture2D* CardImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    FText CardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    int32 PieceHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    int32 PieceAttack;
};
