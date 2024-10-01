#include "DavidGameInstance.h"
#include "Cards/PieceCardData.h"
#include "Cards/SpellCardData.h"
#include "Cards/CardData.h"
#include "Engine/DataTable.h"

void UDavidGameInstance::Init()
{
    Super::Init();

    if(PieceCardsDT == nullptr || SpellCardsDT == nullptr) return;

    TArray<FPieceCardData*> GamePiecesData;
    TArray<FSpellCardData*> GameSpellsData;

    PieceCardsDT->GetAllRows("", GamePiecesData);
    SpellCardsDT->GetAllRows("", GameSpellsData);

    for(const FPieceCardData* PieceData : GamePiecesData)
    {
        GameCards.Add(FCardData(*PieceData));
    }

    for(const FSpellCardData* SpellData : GameSpellsData)
    {
        GameCards.Add(FCardData(*SpellData));
    }
}
