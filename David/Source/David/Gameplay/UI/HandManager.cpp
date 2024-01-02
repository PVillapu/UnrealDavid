#include "HandManager.h"
#include "../Cards/CardData.h"
#include "../UI/CardWidget.h"
#include "Components/Overlay.h"
#include "../Player/DavidPlayerController.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "Engine/DataTable.h"
#include "CardDragDropOperation.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UHandManager::AddCardToHand(const FGameCardData& GameCardData)
{
	// Get CardData associated with the GameCard
	const FCardData* CardData = CardsDataTable->FindRow<FCardData>(GameCardData.CardName, "");
	if (CardData == nullptr) return;

	// Create Card widget
	UCardWidget* NewCard = CreateWidget<UCardWidget>(GetOwningPlayer(), CardWidget);

	// Setup Card Widget
	NewCard->SetupCard(*CardData, GameCardData.CardID);
	CardOverlay->AddChild(NewCard);

	// Add to hand cards array
	HandCards.Add(NewCard);
	
	// Bind delegates
	NewCard->OnHoveredCardDelegate.BindUObject(this, &UHandManager::OnCardHovered);
	NewCard->OnUnhoveredCardDelegate.BindUObject(this, &UHandManager::OnCardUnhovered);
	NewCard->OnGrabbedCardDelegate.BindUObject(this, &UHandManager::OnCardGrabbed);
	NewCard->OnLeftCardDelegate.BindUObject(this, &UHandManager::OnCardLeft); // TODO: IMPORTANT - REMOVE BINDINGS WHEN CARD IS PLAYED OR DESTROYED

	// Calculate cards positions in hand
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

void UHandManager::OnCardGrabbed(UCardWidget& Card, UCardDragDropOperation& CardDragDropOp)
{
	CardDragDropOp.OnCardDrag.AddDynamic(this, &UHandManager::CardDrag);
	HoveredCardIndex = -1;
	Card.SetVisibility(ESlateVisibility::HitTestInvisible);
	Card.SetRenderOpacity(0.3f);
}

void UHandManager::OnCardLeft(UCardWidget& Card, UDragDropOperation& CardDragDropOp)
{
	UCardDragDropOperation* CardDragDrop = Cast<UCardDragDropOperation>(&CardDragDropOp);
	if (CardDragDrop) 
	{
		CardDragDrop->OnCardDrag.RemoveDynamic(this, &UHandManager::CardDrag);
	}

	TryCastCardInBoard(Card);
	CalculateCardsPositions();
}

void UHandManager::CardDrag(UDragDropOperation* Operation, const FPointerEvent& PointerEvent)
{
	// Variable declarations
	FVector WorldPosition, WorldDirection;
	FVector2D PixelPosition, ViewportPosition;
	FHitResult Hit;

	// Get screen space position
	FVector2D ScreenPosition = PointerEvent.GetScreenSpacePosition();
	
	// Transform screen position to viewport position
	USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), ScreenPosition, PixelPosition, ViewportPosition);
	
	// Get viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	else return;

	// Check out of bounds
	if (PixelPosition.X < 0 || PixelPosition.Y < 0 || PixelPosition.X > ViewportSize.X || PixelPosition.Y > ViewportSize.Y)
	{
		LastMouseDragActor = nullptr;
		return;
	}

	UCardDragDropOperation* CardDragDropOp = Cast<UCardDragDropOperation>(Operation);
	if (CardDragDropOp && CardDragDropOp->DraggedCard)
	{
		UCardWidget* DraggedCard = CardDragDropOp->DraggedCard;
		
		FWidgetTransform WidgetTargetTransform(DraggedCard->GetRenderTransform());
		WidgetTargetTransform.Translation = CalculateCardDragPosition(ViewportPosition, ViewportSize);
		DraggedCard->StartRepositioning(WidgetTargetTransform, 0.98f);
	}

	// Deproject viewport position to World in order to cast a line trace
	GetWorld()->GetFirstPlayerController()->DeprojectScreenPositionToWorld(PixelPosition.X, PixelPosition.Y, WorldPosition, WorldDirection);
	
	// Define beginning and end of the line trace
	const FVector Start = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	const FVector End = WorldPosition + WorldDirection * 100000;
	
	// Perform line trace
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Camera);

	// Assign the target actor if hit is valid
	if (Hit.bBlockingHit && IsValid(Hit.GetActor()))
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5, 12, FColor::Purple);
		LastMouseDragActor = Hit.GetActor();
	}
}

void UHandManager::TryCastCardInBoard(UCardWidget& Card)
{	
	// Early return if there is no target
	if (LastMouseDragActor == nullptr)
	{
		ReturnCardToHand(Card);
		return;
	}

	// Cast to board square
	ABoardSquare* Square = Cast<ABoardSquare>(LastMouseDragActor);
		
	// If there is no board square as target, return the card to hand
	if (Square == nullptr) 
	{
		ReturnCardToHand(Card);
		return;
	}

	// Cast the card in the target square
	PlayCardInBoardSquare(Card, Square);
}

void UHandManager::ReturnCardToHand(UCardWidget& Card)
{
	Card.SetRenderOpacity(1.f);
	Card.SetVisibility(ESlateVisibility::Visible);

	CalculateCardsPositions();
}

void UHandManager::PlayCardInBoardSquare(UCardWidget& Card, ABoardSquare* BoardSquare)
{
	UE_LOG(LogTemp, Warning, TEXT("Sending play card request to server"));

	// Hide the card until server confirms or reject the play
	Card.SetRenderOpacity(0.f);
	Card.SetVisibility(ESlateVisibility::Hidden);

	if (UWorld* World = GetWorld()) 
	{
		ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());
		if (DavidPlayerController == nullptr) return;

		// TODO: PlayCardRequest
	}
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

FVector2D UHandManager::CalculateCardDragPosition(const FVector2D ViewportPosition, const FVector2D& ViewportSize) const
{
	FVector2D HandWidgetSize = GetDesiredSize();
	FVector2D Result = ViewportPosition;
	Result.Y -= (ViewportSize.Y - HandWidgetSize.Y);
	
	return Result;
}
