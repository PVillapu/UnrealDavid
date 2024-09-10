#include "PlayerCards.h"
#include "../Cards/CardData.h"
#include "../Player/DavidPlayerController.h"
#include "../UI/HandManager.h"
#include "../UI/GameHUD.h"
#include "../Misc/CustomDavidLogs.h"
#include "../Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "../DavidGameState.h"
#include "../UI/PlayerHUD.h"
#include "../Misc/GameRules.h"

APlayerCards::APlayerCards()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void APlayerCards::SetupPlayerCards()
{
	// Get PlayerController
	PlayerController = Cast<ADavidPlayerController>(GetOwner());

	// Setup player cards reference
	if (PlayerController)
	{
		PlayerController->SetPlayerCards(this);
	}

	// Get BoardManager reference
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
	if (OutActors.Num() > 0)
	{
		BoardManager = Cast<ABoardManager>(OutActors[0]);
	}

	// End Initialization
	PlayerController->InitializationPartDone(EDavidPreMatchInitialization::PLAYER_CARDS_INITIALIZED);
}

void APlayerCards::OnRep_Owner()
{
	SetupPlayerCards();
}

void APlayerCards::SetPlayerDeck(const TArray<int32>& PlayerCards)
{
	GameState = GetWorld()->GetGameState<ADavidGameState>();
	if (GameState == nullptr) return;

	UDataTable* DataTable = GameState->GetCardsDataTable();
	if (DataTable == nullptr) return;

	int32 CardIdCount = 0;
	TArray<FCardData*> CardsArray;
	DataTable->GetAllRows("", CardsArray);
	for(int32 CardIndex : PlayerCards) 
	{
		if (CardIndex < 0 || CardIndex > CardsArray.Num())
		{
			UE_LOG(LogDavid, Warning, TEXT("Invalid card index of the player deck"));
			continue;
		}

		// Get card
		FCardData* CardData = CardsArray[CardIndex];
		if (CardData == nullptr) continue;

		FGameCardData GameCardData;
		GameCardData.CardID = CardIdCount++;
		GameCardData.CardDTIndex = CardIndex;
		GameCardData.PieceAttack = CardData->PieceAttack;
		GameCardData.PieceHealth = CardData->PieceHealth;
		PutCardOnDeck(GameCardData);
	}
}

void APlayerCards::PlayerDrawCards(int32 CardAmmount)
{
	for (int i = 0; i < CardAmmount; ++i) 
	{
		if (PlayerDeckCards.Num() <= 0) break;

		if(PlayerHandCards.Num() >= GameState->GetGameRules()->MaxPlayerHandSize) break;

		FGameCardData DrawCard = PlayerDeckCards[0];
		PlayerDeckCards.RemoveAt(0);
		PlayerHandCards.Add(DrawCard);

		Client_DrawCard(DrawCard);
	}
}

void APlayerCards::PutCardOnDeck(const FGameCardData& Card)
{
	PlayerDeckCards.Add(Card);
}

void APlayerCards::OnPlayerDrawCard(const FGameCardData& GameCardData)
{
	UE_LOG(LogDavid, Display, TEXT("[%s] APlayerCards::OnPlayerDrawCard"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));

	// Catch the PlayerHandManager
	if (PlayerHandManager == nullptr) 
	{
		if (APlayerHUD* PlayerHUD = PlayerController->GetHUD<APlayerHUD>())
		{
			if(UGameHUD* GameHUD = PlayerHUD->GetGameHUDWidget())
				PlayerHandManager = GameHUD->GetPlayerHandManager();
		}
	}

	// Add card
	if(PlayerHandManager)
		PlayerHandManager->AddCardToHand(GameCardData);
}

bool APlayerCards::CheckIfCardCanBePlayed(int32 CardID, int32 SquareID) const
{
	// Check if it is the player turn
	if (!PlayerController->IsPlayerTurn()) return false;

	return BoardManager->CanPlayerPlayCardInSquare(PlayerController->GetDavidPlayer(), SquareID);
}

void APlayerCards::Client_DrawCard_Implementation(FGameCardData GameCardData)
{
	OnPlayerDrawCard(GameCardData);
}

void APlayerCards::Server_PlayCardRequest_Implementation(int32 CardID, int32 SquareID)
{	
	if (CheckIfCardCanBePlayed(CardID, SquareID)) {

		// Get Cards data to complete the request
		FGameCardData* GameCardData = PlayerHandCards.FindByPredicate([CardID](FGameCardData& HandCard) { return HandCard.CardID == CardID; });

		if (GameCardData) 
		{
			EDavidPlayer Player = PlayerController->GetDavidPlayer();

			// Accept the request and play the card
			Client_CardRequestResponse(CardID, true);

			for(int i = 0; i < PlayerHandCards.Num(); ++i)
			{
				if(PlayerHandCards[i].CardID == CardID)
				{
					PlayerHandCards.RemoveAt(i);
					break;
				}
			}

			BoardManager->PlayCardInSquare(*GameCardData, SquareID, Player);
		}
		else 
		{
			// Reject the request
			Client_CardRequestResponse(CardID, false);
		}
	}
	else 
	{
		// Reject the request
		Client_CardRequestResponse(CardID, false);
	}
}

void APlayerCards::Client_CardRequestResponse_Implementation(int32 CardID, bool Response)
{
	PlayerHandManager->OnPlayCardResponse(CardID, Response);
}
