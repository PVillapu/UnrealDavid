#pragma once

#include "CoreMinimal.h"
#include "PieceAction.generated.h"

USTRUCT(BlueprintType)
struct FPieceAction
{
    GENERATED_USTRUCT_BODY()

    /* Default constructor */
    FPieceAction() { }

    FPieceAction(int32 _PieceID, int32 _ActionID)
    {
        PieceID = _PieceID;
        ActionID = _ActionID;
    }

    /* Constructor that copy the given payload into the struct */
    FPieceAction(int32 _PieceID, int32 _ActionID, const TArray<uint8>& DesiredPayload)
    {
        PieceID = _PieceID;
        ActionID = _ActionID;

        Payload.Reserve(DesiredPayload.GetAllocatedSize());
        Payload.SetNum(DesiredPayload.GetAllocatedSize());
        FMemory::Memcpy(Payload.GetData(), DesiredPayload.GetData(), DesiredPayload.GetAllocatedSize());
    }

    UPROPERTY()
    int32 PieceID = -1;

    UPROPERTY()
    int32 ActionID = -1;
    
    UPROPERTY()
    TArray<uint8> Payload;
};
