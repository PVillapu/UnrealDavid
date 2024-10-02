#pragma once

#include "CoreMinimal.h"
#include "SpellAction.generated.h"

USTRUCT(BlueprintType)
struct FSpellAction
{
    GENERATED_USTRUCT_BODY()

    /* Default constructor */
    FSpellAction() { }

    /* Empty payload constructor */
    FSpellAction(int32 _SpellID, int32 _SquareID)
    {
        SpellID = _SpellID;
        SquareID = _SquareID;
    }

    /* Constructor that copy the given payload into the struct */
    FSpellAction(int32 _SpellID, int32 _SquareID, const TArray<uint8>& DesiredPayload)
    {
        SpellID = _SpellID;
        SquareID = _SquareID;

        Payload.Reserve(DesiredPayload.GetAllocatedSize());
        Payload.SetNum(DesiredPayload.GetAllocatedSize());
        FMemory::Memcpy(Payload.GetData(), DesiredPayload.GetData(), DesiredPayload.GetAllocatedSize());
    }
    
    UPROPERTY()
    int32 SpellID;

    UPROPERTY()
    int32 SquareID;

    UPROPERTY()
    TArray<uint8> Payload;
};
