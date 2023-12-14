#include "CardWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "DragWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsInterpolating) 
	{
		if (HasReachedDestination() && HasCardSizeBeenStablished()) 
		{
			bIsInterpolating = false;
		}
		else 
		{
			MoveCardToTarget();
		}
	}
}

//FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
//{
//	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
//	
//	return CustomDetectDrag(InMouseEvent, this, EKeys::LeftMouseButton);
//}

void UCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	OnHoveredCardDelegate.ExecuteIfBound(*this);
}

void UCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	OnUnhoveredCardDelegate.ExecuteIfBound(*this);
}

void UCardWidget::OnCustomDragStart()
{
	SetVisibility(ESlateVisibility::Hidden);

	SetRenderOpacity(0.3f);

	OnGrabbedCardDelegate.ExecuteIfBound(*this);
}

void UCardWidget::OnCustomDragStopped(const UDragDropOperation* DragDropOperation)
{
	SetVisibility(ESlateVisibility::Visible);

	SetRenderOpacity(1.f);

	OnLeftCardDelegate.ExecuteIfBound(*this);
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

	FWidgetTransform newTransform(NewPosition, GetRenderTransform().Scale, GetRenderTransform().Shear, NewRotation);
	SetRenderTransform(newTransform);
}