#include "PlayerCards.h"
#include "../Cards/CardData.h"
#include "../Player/DavidPlayerController.h"
#include "../UI/HandManager.h"
#include "../UI/GameHUD.h"
#include "../Misc/CustomDavidLogs.h"

APlayerCards::APlayerCards()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void APlayerCards::SetupPlayerCards(const TArray<FName>& PlayerCards)
{
	ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(GetOwner());
	if (PlayerController == nullptr) return;

	UDataTable* DataTable = PlayerController->GetCardsDataTable();
	if (DataTable == nullptr) return;

	int32 CardIdCount = 0;
	for(FName CardName : PlayerCards) 
	{
		// Get card
		FCardData* CardData = DataTable->FindRow<FCardData>(CardName, "");
		if (CardData == nullptr) return;

		FGameCardData GameCardData;
		GameCardData.CardID = CardIdCount++;
		GameCardData.CardName = CardName;
		GameCardData.PieceAttack = CardData->PieceAttack;
		GameCardData.PieceHealth = CardData->PieceHealth;
		PlayerDeckCards.Add(GameCardData);
	}
}

void APlayerCards::PlayerDrawCards(int32 CardAmmount)
{
	for (int i = 0; i < CardAmmount; ++i) 
	{
		if (PlayerDeckCards.Num() <= 0) return;

		FGameCardData DrawCard = PlayerDeckCards[0];
		PlayerDeckCards.RemoveAt(0);
		PlayerHandCards.Add(DrawCard);

		Client_DrawCard(DrawCard);
	}
}

void APlayerCards::OnPlayerDrawCard(const FGameCardData& GameCardData)
{
	ADavidPlayerController* PlayerController = Cast<ADavidPlayerController>(GetOwner());
	if (PlayerController == nullptr) return;

	UE_LOG(LogDavid, Display, TEXT("[%s] APlayerCards::OnPlayerDrawCard"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));

	// Catch the PlayerHandManager
	if (PlayerHandManager == nullptr) 
	{
		if(UGameHUD* PlayerHUD = PlayerController->GetPlayerGameHUD())
			PlayerHandManager = PlayerHUD->GetPlayerHandManager();
	}

	// Add card
	if(PlayerHandManager)
		PlayerHandManager->AddCardToHand(GameCardData);
}

void APlayerCards::Client_DrawCard_Implementation(FGameCardData GameCardData)
{
	PlayerHandCards.Add(GameCardData);

	OnPlayerDrawCard(GameCardData);
}

void APlayerCards::Server_PlayCardRequest_Implementation(int32 CardID)
{
}

void APlayerCards::Client_CardRequestResponse_Implementation(int32 CardID, bool Approved)
{
}
