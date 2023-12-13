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
	NewCard->OnHoveredCardDelegate.BindUObject(this, &UHandManager::OnCardHovered);
	NewCard->OnUnhoveredCardDelegate.BindUObject(this, &UHandManager::OnCardUnhovered);	// TODO: IMPORTANT - REMOVE BINDINGS WHEN CARD IS PLAYED OR DESTROYED

	CalculateCardsPositions();
}

float UHandManager::GetCardAngle(int CardIndex) const
{
	return GetCardIndexFromCenter(CardIndex) * CardAngle;
}

FVector2D UHandManager::GetCardPosition(int CardIndex) const
{
	return FVector2D((CardIndex - HandCards.Num() / 2.f) * CardSpacing + HandCardsHorizontalOffset + GetHoveredXDisplacement(CardIndex), HandCardsVerticalOffset + GetHoveredCardYDisplacement(CardIndex));
}

void UHandManager::CalculateCardsPositions() const
{
	for (int i = 0; i < HandCards.Num(); ++i)
	{
		HandCards[i]->StartRepositioning(CalculateCardPosition(i), CardInterpSpeed);
	}
}

void UHandManager::OnCardHovered(UCardWidget& Card)
{
	int32 CardIndex = -1;
	if (!HandCards.Find(&Card, CardIndex)) return;

	HoveredCardIndex = CardIndex;
	CalculateCardsPositions();
}

void UHandManager::OnCardUnhovered(UCardWidget& Card)
{
	int32 CardIndex = -1;
	if (!HandCards.Find(&Card, CardIndex)) return;

	if (CardIndex == HoveredCardIndex) HoveredCardIndex = -1;
	
	CalculateCardsPositions();
}

FWidgetTransform UHandManager::CalculateCardPosition(int CardIndex) const
{
	return FWidgetTransform(GetCenterPosition() + GetCardPosition(CardIndex) + GetCardHandHeight(CardIndex),
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
	float RawIndex = CardIndex - HandCards.Num() / 2.f;
	int res = FMath::RoundHalfFromZero(RawIndex + 0.5f);

	return res;
}

FVector2D UHandManager::GetCardHandHeight(int CardIndex) const
{
	return FVector2D(0.0, FMath::Abs(GetCardIndexFromCenter(CardIndex)) * CardHeight);
}

float UHandManager::GetHoveredCardYDisplacement(int CardIndex) const
{
	return CardIndex == HoveredCardIndex ? HoveredCardYDisplacement : 0.f;
}

float UHandManager::GetHoveredXDisplacement(int CardIndex) const
{
	return HoveredCardIndex >= 0 && CardIndex > HoveredCardIndex ? HoveredCardXDisplacement : 0.f;
}