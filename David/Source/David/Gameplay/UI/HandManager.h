#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HandManager.generated.h"

/**
 * Handles the player hand
 */
UCLASS()
class DAVID_API UHandManager : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeHandManager();

	void AddCardToHand(const struct FGameCardData& Card);

	void OnPlayCardResponse(int32 CardID, bool Response);

private:
	void CalculateCardsPositions() const;

	void OnCardHovered(class UCardWidget& Card);

	void OnCardUnhovered(UCardWidget& Card);

	void OnCardGrabbed(UCardWidget& Card, class UCardDragDropOperation& CardDragDropOp);

	void OnCardLeft(UCardWidget& Card, UDragDropOperation& CardDragDropOp);

	UFUNCTION()
	void CardDrag(UDragDropOperation* Operation, const FPointerEvent& PointerEvent);

	void TryCastCardInBoard(UCardWidget& Card);

	void ReturnCardToHand(UCardWidget& Card);

	void PlayCardInBoardSquare(UCardWidget& Card, class ABoardSquare* BoardSquare);

	FWidgetTransform CalculateCardPosition(int CardIndex) const;

	float GetCardAngle(int CardIndex) const;

	FVector2D GetCardPosition(int CardIndex) const;

	FVector2D GetCenterPosition() const;

	int GetCardIndexFromCenter(int CardIndex) const;

	FVector2D GetCardHandHeight(int CardIndex) const;

	float GetHoveredCardYDisplacement(int CardIndex) const;

	float GetHoveredXDisplacement(int CardIndex) const;

	FVector2D CalculateCardDragPosition(const FVector2D& ViewportPosition, const UWidget* DraggedCard) const;

	UCardWidget* GetAvailableCardWidget();

	void ReturnCardWidget(UCardWidget* CardWidget);

protected:
	UPROPERTY(EditAnywhere, Category = "David")
	class UDataTable* CardsDataTable;

	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<class UUserWidget> CardWidgetClass;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UOverlay* CardOverlay;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardAngle = 10.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardInterpSpeed = 0.3f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardHeight = 10.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardSpacing = 100.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HandCardsHorizontalOffset = 100.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HandCardsVerticalOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HoveredCardYDisplacement = -100.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HoveredCardXDisplacement = 100.f;

private:
	int HoveredCardIndex = -1;

	UPROPERTY(Transient, SkipSerialization)
	AActor* LastMouseDragActor;

	UPROPERTY(Transient, SkipSerialization)
	TArray<UCardWidget*> AvailableCardWidgets;

	UPROPERTY(Transient, SkipSerialization)
	TArray<class UCardWidget*> HandCards;

	UPROPERTY(Transient, SkipSerialization)
	FVector2D HandsSlotOffset;
};
