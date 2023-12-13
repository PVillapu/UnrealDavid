#include "CardWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCardWidget::SetupCard(const FCardData& Data)
{
	PieceImage->SetBrushFromTexture(Data.CardImage);
	HealthText->SetText(FText::AsNumber(Data.PieceHealth));
	AttackText->SetText(FText::AsNumber(Data.PieceAttack));
	CardDescriptionText->SetText(Data.CardDescription);

	CardData = Data;
}

void UCardWidget::StartRepositioning(const FWidgetTransform& TargetTransform, float InterpSpeed)
{
	TargetWidgetTransform = TargetTransform;
	InterpolationSpeed = InterpSpeed;

	bIsInterpolating = true;
}

void UCardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if (bIsInterpolating) 
	{
		if (HasReachedDestination() && HasCardSizeBeenStablished()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("bIsInterpolating = false"));
			bIsInterpolating = false;
		}
		else 
		{
			MoveCardToTarget();
		}
	}
}

bool UCardWidget::HasReachedDestination()
{
	return (TargetWidgetTransform.Translation - GetRenderTransform().Translation).Length() < 0.1f
		&& FMath::Abs(TargetWidgetTransform.Angle - GetRenderTransform().Angle) < 0.1f;
}

bool UCardWidget::HasCardSizeBeenStablished()
{
	return GetCachedGeometry().GetLocalSize().X != 0 && GetCachedGeometry().GetLocalSize().Y != 0;
}

void UCardWidget::MoveCardToTarget()
{
	FVector2D NewPosition = FMath::Lerp(GetRenderTransform().Translation, TargetWidgetTransform.Translation, InterpolationSpeed);
	float NewRotation = FMath::Lerp(GetRenderTransform().Angle, TargetWidgetTransform.Angle, InterpolationSpeed);

	FWidgetTransform newTransform(NewPosition, RenderTransform.Scale, RenderTransform.Shear, NewRotation);
	SetRenderTransform(newTransform);
}
