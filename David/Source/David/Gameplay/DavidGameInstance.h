#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DavidGameInstance.generated.h"


UCLASS()
class DAVID_API UDavidGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	FORCEINLINE class UDataTable* GetPieceCardsDataTable() const { return PieceCardsDT; }

	FORCEINLINE UDataTable* GetSpellCardsDataTable() const { return SpellCardsDT; }

	FORCEINLINE TArray<struct FCardData>& GetGameCards() { return GameCards; }

	void Init() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "David|Cards")
	UDataTable* PieceCardsDT;

	UPROPERTY(EditDefaultsOnly, Category = "David|Cards")
	UDataTable* SpellCardsDT;

	UPROPERTY(Transient, SkipSerialization)
	TArray<FCardData> GameCards;
};
