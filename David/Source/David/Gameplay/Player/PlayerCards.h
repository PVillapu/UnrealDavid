#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Cards/GameCardData.h"
#include "PlayerCards.generated.h"

UCLASS()
class DAVID_API APlayerCards : public AActor
{
	GENERATED_BODY()

public:
	APlayerCards();

	void SetupPlayerCards();

	void SetPlayerDeck(const TArray<FName>& PlayerCards);

	void PlayerDrawCards(int32 CardAmmount);

	UFUNCTION(Client, reliable)
	void Client_DrawCard(FGameCardData GameCardData);

	UFUNCTION(Server, reliable)
	void Server_PlayCardRequest(int32 CardID, int32 SquareID);

	UFUNCTION(Client, reliable)
	void Client_CardRequestResponse(int32 CardID, bool Approved);

protected:
	void OnRep_Owner() override;

private:
	void OnPlayerDrawCard(const FGameCardData& GameCardData);

	bool CheckIfCardCanBePlayed(int32 CardID, int32 SquareID) const;

private:
	UPROPERTY(Transient, SkipSerialization)
	class ADavidPlayerController* PlayerController;

	UPROPERTY(Transient, SkipSerialization)
	TArray<FGameCardData> PlayerHandCards;

	UPROPERTY(Transient, SkipSerialization)
	TArray<FGameCardData> PlayerDeckCards;

	UPROPERTY(Transient, SkipSerialization)
	class UHandManager* PlayerHandManager;

	UPROPERTY(Transient, SkipSerialization)
	class ABoardManager* BoardManager;
};
