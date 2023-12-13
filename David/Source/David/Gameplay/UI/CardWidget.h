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
	bool HasReachedDestination();

	bool HasCardSizeBeenStablished();

	void MoveCardToTarget();

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
