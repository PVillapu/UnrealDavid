#include "HandManager.h"
#include "../Cards/CardData.h"
#include "../UI/CardWidget.h"
#include "Components/Overlay.h"

void UHandManager::AddCardToHand(const FCardData& Card)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			3,
			15.f,
			FColor::Yellow,
			TEXT("Added a random card to your hand!")
		);
	}

	UCardWidget* NewCard = CreateWidget<UCardWidget>(GetOwningPlayer(), CardWidget);
	NewCard->SetupCard(Card);
	CardOverlay->AddChild(NewCard);

	HandCards.Add(NewCard);

	for (int i = 0; i < HandCards.Num(); ++i) 
	{
		HandCards[i]->StartRepositioning(CalculateCardPosition(i), CardInterpSpeed);
	}
}

float UHandManager::GetCardAngle(int CardIndex) const
{
	return GetCardIndexFromCenter(CardIndex) * CardAngle;
}

FVector2D UHandManager::GetCardPosition(int CardIndex) const
{
	return FVector2D(CardIndex * CardSpacing, 0.f);
}

FWidgetTransform UHandManager::CalculateCardPosition(int CardIndex) const
{
	return FWidgetTransform(GetCardPosition(CardIndex) + GetCenterPosition() + GetCardHandHeight(CardIndex) + FVector2D(HandCardsHorizontalOffset, HandCardsVerticalOffset),
		FVector2D::One(),
		FVector2D::Zero(),
		GetCardAngle(CardIndex));
}

FVector2D UHandManager::GetCenterPosition() const
{
	return FVector2D(GetCachedGeometry().GetLocalSize().X / 2.0, 0.0);
}

int UHandManager::GetCardIndexFromCenter(int CardIndex) const
{
	return (CardIndex - FMath::FloorToInt(HandCards.Num() / 2.f));
}

FVector2D UHandManager::GetCardHandHeight(int CardIndex) const
{
	return FVector2D(0.0, FMath::Abs(GetCardIndexFromCenter(CardIndex)) * CardHeight);
}
