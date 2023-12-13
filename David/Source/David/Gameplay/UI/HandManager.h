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
	
protected:
	UPROPERTY(EditAnywhere, Category = "David")
	TSubclassOf<class UUserWidget> CardWidget;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UOverlay* CardOverlay;

	UPROPERTY()
	TArray<class UCardWidget*> HandCards;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardAngle = 10.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardInterpSpeed = 0.3f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardHeight = 10.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float CardSpacing = 100.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HandCardsHorizontalOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "David")
	float HandCardsVerticalOffset = 0.f;

public:
	UFUNCTION(BlueprintCallable)
	void AddCardToHand(const struct FCardData& Card);

private:
	FWidgetTransform CalculateCardPosition(int CardIndex) const;

	float GetCardAngle(int CardIndex) const;

	FVector2D GetCardPosition(int CardIndex) const;

	FVector2D GetCenterPosition() const;

	int GetCardIndexFromCenter(int CardIndex) const;

	FVector2D GetCardHandHeight(int CardIndex) const;
};
