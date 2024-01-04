#pragma once

#include "CoreMinimal.h"
#include "PieceAction.generated.h"

USTRUCT(BlueprintType)
struct FPieceAction
{
    GENERATED_USTRUCT_BODY()

    int32 PieceID;
    int32 ActionID;
    //Payload TODO
};
