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

	int GetCardIndexFromCenter(int CardIndex) const;

	float GetHoveredCardYDisplacement(int CardIndex) const;

	float GetHoveredXDisplacement(int CardIndex) const;

	FVector2D CalculateCardDragPosition(const FVector2D& ViewportPosition, const UWidget* DraggedCard) const;

	UCardWidget* GetAvailableCardWidget();

	void ReturnCardWidget(UCardWidget* CardWidget);

protected:
	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<class UUserWidget> CardWidgetClass;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UCanvasPanel* CardCanvasPanel;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardAnglePercentage = 0.1f;

	UPROPERTY(EditAnywhere, Category = "David")
	float MaxCardAngle = 75.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardInterpSpeed = 0.3f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardSpacing = 10.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardHeightPercentage = 0.1f;

private:
	UPROPERTY(Transient, SkipSerialization)
	AActor* LastMouseDragActor;

	UPROPERTY(Transient, SkipSerialization)
	TArray<UCardWidget*> AvailableCardWidgets;

	UPROPERTY(Transient, SkipSerialization)
	TArray<class UCardWidget*> HandCards;

	int HoveredCardIndex = -1;
};
