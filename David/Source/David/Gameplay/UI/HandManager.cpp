#include "HandManager.h"
#include "../Cards/CardData.h"
#include "../UI/CardWidget.h"
#include "Components/CanvasPanel.h"
#include "../Player/DavidPlayerController.h"
#include "../Board/BoardSquare.h"
#include "../Board/BoardManager.h"
#include "../Cards/GameCardData.h"
#include "Engine/DataTable.h"
#include "CardDragDropOperation.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "../Player/PlayerCards.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

void UHandManager::InitializeHandManager()
{
	FGeometry ParentGeometry = Slot->Parent->GetCachedGeometry();
	HandsSlotOffset = ParentGeometry.AbsoluteToLocal(GetCachedGeometry().GetAbsolutePosition());
}

void UHandManager::AddCardToHand(const FGameCardData& GameCardData)
{
	// Get CardData associated with the GameCard
	const FCardData* CardData = CardsDataTable->FindRow<FCardData>(GameCardData.CardName, "");
	if (CardData == nullptr) return;

	// Create Card widget
	UCardWidget* NewCard = GetAvailableCardWidget();
	if (NewCard == nullptr) return;

	// Setup Card Widget
	NewCard->SetupCard(*CardData, GameCardData.CardID);

	// Add to hand cards array
	HandCards.Add(NewCard);

	// Calculate cards positions in hand
	CalculateCardsPositions();
}

float UHandManager::GetCardAngle(int CardIndex) const
{
	return GetCardIndexFromCenter(CardIndex) * CardAngle;
}

FVector2D UHandManager::GetCardPosition(int CardIndex) const
{
	return FVector2D((CardIndex - HandCards.Num() / 2.f) * CardSpacing + GetHoveredXDisplacement(CardIndex), GetHoveredCardYDisplacement(CardIndex));
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
	Card.SetIsBeingGrabbed(true);
}

void UHandManager::OnCardLeft(UCardWidget& Card, UDragDropOperation& CardDragDropOp)
{
	Card.SetIsBeingGrabbed(false);

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
		
		// Calculate the card position in viewport
		FVector2D CardPosition = CalculateCardDragPosition(ViewportPosition, DraggedCard);

		FWidgetTransform WidgetTargetTransform(DraggedCard->GetRenderTransform());
		WidgetTargetTransform.Translation = CardPosition;
		WidgetTargetTransform.Angle = 0.f;
		DraggedCard->SetRenderTransform(WidgetTargetTransform);
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
	// Make the card visible again
	Card.SetRenderOpacity(1.f);
	Card.SetVisibility(ESlateVisibility::Visible);

	CalculateCardsPositions();
}

void UHandManager::PlayCardInBoardSquare(UCardWidget& Card, ABoardSquare* BoardSquare)
{
	// Hide the card until server confirms or reject the play
	Card.SetRenderOpacity(0.f);
	Card.SetVisibility(ESlateVisibility::Hidden);

	if (UWorld* World = GetWorld()) 
	{
		ADavidPlayerController* DavidPlayerController = Cast<ADavidPlayerController>(World->GetFirstPlayerController());
		if (DavidPlayerController == nullptr && DavidPlayerController->GetPlayerCards()) return;
		
		// Send the play card request to the server
		DavidPlayerController->GetPlayerCards()->Server_PlayCardRequest(Card.GetGameCardID(), BoardSquare->GetSquareIndex());
	}
}

void UHandManager::OnPlayCardResponse(int32 CardID, bool Response)
{
	// Search the response target Card
	UCardWidget* Card = *HandCards.FindByPredicate([CardID](UCardWidget* HandCard) { return HandCard->GetGameCardID() == CardID; });
	if (Card == nullptr) return;

	if (Response) 
	{
		// If card was played, remove from hand
		HandCards.Remove(Card);	
		ReturnCardWidget(Card);
	}
	else
	{
		// If card was rejected, return to the player hand
		ReturnCardToHand(*Card);
	}
}

FWidgetTransform UHandManager::CalculateCardPosition(int CardIndex) const
{
	return FWidgetTransform(GetCardPosition(CardIndex),
		FVector2D::One(),
		FVector2D::Zero(),
		GetCardAngle(CardIndex));
}

int UHandManager::GetCardIndexFromCenter(int CardIndex) const
{
	float RawIndex = CardIndex - HandCards.Num() / 2.f;

	return (int)FMath::RoundHalfFromZero(RawIndex + 0.5f);
}

float UHandManager::GetHoveredCardYDisplacement(int CardIndex) const
{
	if (HandCards.Num() <= 0) return 0.f;
	
	const FVector2D CardSize = HandCards[0]->GetCardSize();
	
	if(HoveredCardIndex == -1 || CardIndex != HoveredCardIndex) return -CardSize.Y / 2.f;

	return -CardSize.Y;
}

float UHandManager::GetHoveredXDisplacement(int CardIndex) const
{
	return HoveredCardIndex >= 0 && CardIndex > HoveredCardIndex ? /* HoveredCardXDisplacement */ 10 : 0.f;
}

FVector2D UHandManager::CalculateCardDragPosition(const FVector2D& ViewportPosition, const UWidget* DraggedCard) const
{	
	return ViewportPosition - HandsSlotOffset - DraggedCard->GetCachedGeometry().GetLocalSize() * 0.5f;
}

UCardWidget* UHandManager::GetAvailableCardWidget()
{
	UCardWidget* CardWidget;

	if (AvailableCardWidgets.Num() > 0)
	{
		// Get Card widget from the available list
		CardWidget = AvailableCardWidgets[0];
		AvailableCardWidgets.Remove(CardWidget);
	}
	else 
	{
		// Create Card widget
		CardWidget = CreateWidget<UCardWidget>(GetOwningPlayer(), CardWidgetClass);
		CardCanvasPanel->AddChild(CardWidget);

		UCanvasPanelSlot* WidgetAsPanelSlot = Cast<UCanvasPanelSlot>(CardWidget->Slot);
		if (WidgetAsPanelSlot) 
		{
			WidgetAsPanelSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		}
	}

	if (CardWidget == nullptr) return nullptr;
	
	// Enable widget
	CardWidget->SetRenderOpacity(1);
	CardWidget->SetVisibility(ESlateVisibility::Visible);

	// Bind delegates
	CardWidget->OnHoveredCardDelegate.BindUObject(this, &UHandManager::OnCardHovered);
	CardWidget->OnUnhoveredCardDelegate.BindUObject(this, &UHandManager::OnCardUnhovered);
	CardWidget->OnGrabbedCardDelegate.BindUObject(this, &UHandManager::OnCardGrabbed);
	CardWidget->OnLeftCardDelegate.BindUObject(this, &UHandManager::OnCardLeft);

	return CardWidget;
}

void UHandManager::ReturnCardWidget(UCardWidget* CardWidget)
{
	// Check if it is already in the available list
	if (AvailableCardWidgets.Contains(CardWidget)) return;

	// Disable it
	CardWidget->SetRenderOpacity(0);
	CardWidget->SetVisibility(ESlateVisibility::Hidden);

	// Unbind delegates
	CardWidget->OnHoveredCardDelegate.Unbind();
	CardWidget->OnUnhoveredCardDelegate.Unbind();
	CardWidget->OnGrabbedCardDelegate.Unbind();
	CardWidget->OnLeftCardDelegate.Unbind();
	
	// Return to the list of available CardWidgets
	AvailableCardWidgets.Add(CardWidget);
}
