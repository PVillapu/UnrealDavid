#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SpellCardData.generated.h"

USTRUCT(BlueprintType)
struct FSpellCardData : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    FText CardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    UTexture2D* CardImage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    FText CardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    int32 CardCost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "David")
    TSubclassOf<class UDavidSpell> CardSpellClass;
};
