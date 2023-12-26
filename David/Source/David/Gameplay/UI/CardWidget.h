#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Cards/CardData.h"
#include "CardWidget.generated.h"

/**
 * Handles a card UI
 */
UCLASS()
class DAVID_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetupCard(const FCardData& Data, const FName RowName, TSubclassOf<class UUserWidget>& CardWidget, class UOverlay* CardOverlay = nullptr);

	void StartRepositioning(const FWidgetTransform& TargetTransform, float InterpSpeed);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	FORCEINLINE FCardData* GetCardData() { return &CardData; }

	FORCEINLINE void SetCardDataRowName(const FName CardRowName) { CardDataRowName = CardRowName; }

	FORCEINLINE FName GetCardDataRowName() { return CardDataRowName; }

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

private:
	bool HasReachedDestination();

	bool HasCardSizeBeenStablished();

	void MoveCardToTarget();

public:
	// Delegate called when player is hovering this card
	DECLARE_DELEGATE_OneParam(FOnHoveredCard, UCardWidget&)
	FOnHoveredCard OnHoveredCardDelegate;
	
	// Delegate called when player stops hovering this card
	DECLARE_DELEGATE_OneParam(FOnUnhoveredCard, UCardWidget&)
	FOnUnhoveredCard OnUnhoveredCardDelegate;

	// Delegate called when player is grabbing this card
	DECLARE_DELEGATE_TwoParams(FOnGrabbedCard, UCardWidget&, class UCardDragDropOperation&)
	FOnGrabbedCard OnGrabbedCardDelegate;

	// Delegate called when player stops grabbing this card
	DECLARE_DELEGATE_TwoParams(FOnLeftCard, UCardWidget&, UDragDropOperation&)
	FOnLeftCard OnLeftCardDelegate;

protected:
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UImage* PieceImage;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* AttackText;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* CardDescriptionText;

	UPROPERTY(EditAnywhere, Category = "David")
	FKey CardDragKey;

	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<UDragDropOperation> DragDropOperationBP;

	UPROPERTY(BlueprintReadOnly, Category = "David")
	FCardData CardData;

	UPROPERTY(BlueprintReadOnly, Category = "David")
	FName CardDataRowName;

	UPROPERTY(Transient, SkipSerialization)
	class UOverlay* CardOverlay;

	UPROPERTY(Transient, SkipSerialization)
	TSubclassOf<class UUserWidget> CardWidget;

private:
	float InterpolationSpeed;
	FWidgetTransform TargetWidgetTransform;
	bool bIsInterpolating;
};
