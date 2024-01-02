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

	void SetupPlayerCards(const TArray<FName>& PlayerCards);

	void PlayerDrawCards(int32 CardAmmount);

	UFUNCTION(Server, reliable)
	void Server_DrawCard(FGameCardData GameCardData);

	UFUNCTION(Server, reliable)
	void Server_PlayCardRequest(int32 CardID);

	UFUNCTION(Client, reliable)
	void Client_CardRequestResponse(int32 CardID, bool Approved);

private:
	void OnPlayerDrawCard(const FGameCardData& GameCardData);

private:
	UPROPERTY(Transient, SkipSerialization)
	TArray<FGameCardData> PlayerHandCards;

	UPROPERTY(Transient, SkipSerialization)
	TArray<FGameCardData> PlayerDeckCards;

	UPROPERTY(Transient, SkipSerialization)
	class UHandManager* PlayerHandManager;
};
