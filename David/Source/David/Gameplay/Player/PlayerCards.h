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

	void SetPlayerDeck(const TArray<int32>& PlayerCards);

	void PlayerDrawCards(int32 CardAmmount);

	void PutCardOnDeck(const FGameCardData& Card);

	void AddCardToPlayerHand(int32 CardId);

	void CacheGameCards();

	bool CreateNewCardForPlayer(int32 CardId, FGameCardData& NewGameCardData);

	UFUNCTION(Client, reliable)
	void Client_DrawCard(FGameCardData GameCardData);

	UFUNCTION(Client, reliable)
	void Client_AddCardToHand(FGameCardData GameCardData);

	UFUNCTION(Server, reliable)
	void Server_PlayCardRequest(int32 CardID, int32 SquareID);

	UFUNCTION(Client, reliable)
	void Client_CardRequestResponse(int32 CardID, bool Approved);

protected:
	void OnRep_Owner() override;

private:
	void OnPlayerDrawCard(const FGameCardData& GameCardData);

	void OnPlayerAddCardToHand(const FGameCardData& GameCardData);

	bool CheckIfCardCanBePlayed(int32 CardID) const;

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

	TArray<struct FCardData>* CardsArray;

	int32 GameCardsIndexCount = 0;
};
