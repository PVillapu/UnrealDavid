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
	UFUNCTION(BlueprintCallable, Category = "David")
	void SetupCard(const FCardData& Data);

	void StartRepositioning(const FWidgetTransform& TargetTransform, float InterpSpeed);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "David")
	void OnCustomDragStart();

	UFUNCTION(BlueprintCallable, Category = "David")
	void OnCustomDragStopped(const UDragDropOperation* DragDropOperation);

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
	DECLARE_DELEGATE_OneParam(FOnGrabbedCard, UCardWidget&)
	FOnGrabbedCard OnGrabbedCardDelegate;

	// Delegate called when player stops grabbing this card
	DECLARE_DELEGATE_OneParam(FOnLeftCard, UCardWidget&)
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "David")
	FCardData CardData;

private:
	float InterpolationSpeed;
	FWidgetTransform TargetWidgetTransform;
	bool bIsInterpolating;
};
