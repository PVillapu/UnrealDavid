#pragma once

#include "CoreMinimal.h"
#include "../Misc/Enums.h"
#include "SquareAction.generated.h"

USTRUCT()
struct FSquareAction
{
    GENERATED_USTRUCT_BODY()

    /* Default constructor */
    FSquareAction() { }

    FSquareAction(int32 _PieceID, int32 _ActionID)
    {
        SquareIndex = _PieceID;
        ActionID = _ActionID;
    }

    /* Constructor that copy the given payload into the struct */
    FSquareAction(int32 _PieceID, int32 _ActionID, const TArray<uint8>& DesiredPayload)
    {
        SquareIndex = _PieceID;
        ActionID = _ActionID;

        Payload.Reserve(DesiredPayload.GetAllocatedSize());
        Payload.SetNum(DesiredPayload.GetAllocatedSize());
        FMemory::Memcpy(Payload.GetData(), DesiredPayload.GetData(), DesiredPayload.GetAllocatedSize());
    }

    UPROPERTY()
    int32 SquareIndex;

    UPROPERTY()
    int32 ActionID;

    UPROPERTY()
    TArray<uint8> Payload;
};