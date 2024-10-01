#pragma once

#include "CoreMinimal.h"
#include "../Misc/Enums.h"
#include "TurnAction.generated.h"

USTRUCT()
struct FTurnAction
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TEnumAsByte<EDavidGameAction> ActionType = EDavidGameAction::PLAY_CARD;

    UPROPERTY()
    TArray<uint8> Payload;
};