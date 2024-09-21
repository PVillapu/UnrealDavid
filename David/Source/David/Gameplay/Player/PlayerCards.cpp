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
#include "../DavidPlayerState.h"

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
	for(int32 CardIndex : PlayerCards) 
	{
		if (CardIndex < 0 || CardIndex > CardsArray.Num())
		{
			UE_LOG(LogDavid, Warning, TEXT("Invalid card index of the player deck"));
			continue;
		}

		FGameCardData NewGameCardData;
		if(CreateNewCardForPlayer(CardIndex, NewGameCardData))
		{
			PutCardOnDeck(NewGameCardData);
		}
	}
}

void APlayerCards::PlayerDrawCards(int32 CardAmmount)
{
	for (int i = 0; i < CardAmmount; ++i) 
	{
		if (PlayerDeckCards.Num() <= 0) break;

		if(PlayerHandCards.Num() >= GameState->GetGameRules()->MaxPlayerHandSize) break;

		FGameCardData CardDrawn = PlayerDeckCards[0];
		PlayerDeckCards.RemoveAt(0);
		PlayerHandCards.Add(CardDrawn);

		Client_DrawCard(CardDrawn);
	}
}

void APlayerCards::PutCardOnDeck(const FGameCardData& Card)
{
	PlayerDeckCards.Add(Card);
}

void APlayerCards::AddCardToPlayerHand(int32 CardId)
{
	if(PlayerHandCards.Num() >= GameState->GetGameRules()->MaxPlayerHandSize) return;

	FGameCardData NewPlayerCard;
	if(CreateNewCardForPlayer(CardId, NewPlayerCard))
	{
		PlayerHandCards.Add(NewPlayerCard);
		Client_AddCardToHand(NewPlayerCard);
	}
}

void APlayerCards::CacheGameCards()
{
	if(CardsArray.Num() > 0) return;

	GameState = GetWorld()->GetGameState<ADavidGameState>();
	if (GameState == nullptr) return;

	UDataTable* DataTable = GameState->GetCardsDataTable();
	if (DataTable == nullptr) return;

	DataTable->GetAllRows("", CardsArray);
}

bool APlayerCards::CreateNewCardForPlayer(int32 CardId, FGameCardData& NewGameCardData)
{
	CacheGameCards();

	FCardData* CardData = CardsArray[CardId];

	if(CardData == nullptr) return false;

	NewGameCardData = FGameCardData(*CardData);
	NewGameCardData.CardID = CardsIndexCount++;
	NewGameCardData.CardDTIndex = CardId;

	return true;
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

void APlayerCards::OnPlayerAddCardToHand(const FGameCardData &GameCardData)
{
	UE_LOG(LogDavid, Display, TEXT("[%s] APlayerCards::OnPlayerAddCardToHand"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));

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

void APlayerCards::Client_DrawCard_Implementation(FGameCardData GameCardData)
{
	OnPlayerDrawCard(GameCardData);
}

void APlayerCards::Server_PlayCardRequest_Implementation(int32 CardID, int32 SquareID)
{		
	if (CheckIfCardCanBePlayed(CardID, SquareID)) {

		// Get Cards data to complete the request
		FGameCardData* GameCardData = PlayerHandCards.FindByPredicate([CardID](FGameCardData& HandCard) { return HandCard.CardID == CardID; });

		if(GameCardData == nullptr)
		{
			UE_LOG(LogDavid, Display, TEXT("[%s] Cannot play card because CardID was not found"), GetLocalRole() == ROLE_Authority ? *FString("Server") : *FString("Client"));
			Client_CardRequestResponse(CardID, false);
			return;
		}

		// Check if player has enough gold to play the card
		ADavidPlayerState* PlayerState = PlayerController->GetPlayerState<ADavidPlayerState>();
		if(PlayerState) 
		{
#if UE_WITH_CHEAT_MANAGER
			if(!GameState->GetInfiniteGoldStatus())
			{
#endif

			const int32 PlayerGold = PlayerState->GetPlayerGold();
			if(PlayerGold < GameCardData->CardCost)
			{
				Client_CardRequestResponse(CardID, false);
				return;
			}

#if UE_WITH_CHEAT_MANAGER
			}
#endif
		}

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

void APlayerCards::Client_AddCardToHand_Implementation(FGameCardData GameCardData)
{
	OnPlayerAddCardToHand(GameCardData);
}

bool APlayerCards::CheckIfCardCanBePlayed(int32 CardID, int32 SquareID) const
{
	// Check if it is the player turn
	if (!PlayerController->IsPlayerTurn()) return false;

	return BoardManager->CanPlayerPlayCardInSquare(PlayerController->GetDavidPlayer(), SquareID);
}