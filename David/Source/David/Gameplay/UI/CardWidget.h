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
	void SetupCard(const FCardData& Data);

	void StartRepositioning(const FWidgetTransform& TargetTransform, float InterpSpeed);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	bool HasReachedDestination();

	bool HasCardSizeBeenStablished();

	void MoveCardToTarget();

public:
	// Delegate called when player is hovering this card
	DECLARE_DELEGATE_OneParam(FOnHoveredCard, UCardWidget&)
	FOnHoveredCard OnHoveredCardDelegate;
	
	// Delegate called when player is hovering this card
	DECLARE_DELEGATE_OneParam(FOnUnhoveredCard, UCardWidget&)
	FOnUnhoveredCard OnUnhoveredCardDelegate;

protected:
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UImage* PieceImage;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* AttackText;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* CardDescriptionText;

	UPROPERTY()
	FCardData CardData;

private:
	float InterpolationSpeed;
	FWidgetTransform TargetWidgetTransform;
	bool bIsInterpolating;
};
