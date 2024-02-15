#include "CardWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "DragWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CardDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Overlay.h"
#include "../Cards/CardData.h"

void UCardWidget::SetupCard(const FCardData& Data, int32 GameCardID)
{
	PieceImage->SetBrushFromTexture(Data.CardImage);
	CardNameText->SetText(Data.CardName);
	HealthText->SetText(FText::AsNumber(Data.PieceHealth));
	AttackText->SetText(FText::AsNumber(Data.PieceAttack));
	CardDescriptionText->SetText(Data.CardDescription);

	CardID = GameCardID;
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

	if (bIsInterpolating && !bIsBeingGrabbed)
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

void UCardWidget::SetCardAttack(int32 Attack)
{
	AttackText->SetText(FText::AsNumber(Attack));
}

void UCardWidget::SetCardHealth(int32 Health)
{
	HealthText->SetText(FText::AsNumber(Health));
}

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

FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == CardDragKey /*|| PointerEvent.IsTouchEvent()*/)
	{
		FEventReply Reply;
		Reply.NativeReply = FReply::Handled();

		TSharedPtr<SWidget> SlateWidgetDetectingDrag = GetCachedWidget();
		if (SlateWidgetDetectingDrag.IsValid())
		{
			Reply.NativeReply = Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), CardDragKey);
			return Reply.NativeReply;
		}
	}

	return FReply::Unhandled();
}

void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UCardDragDropOperation* DragDropOp = Cast<UCardDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(DragDropOperationBP));

	DragDropOp->DraggedCard = this;

	OutOperation = DragDropOp;

	OnGrabbedCardDelegate.ExecuteIfBound(*this, *DragDropOp);
}

void UCardWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	OnLeftCardDelegate.ExecuteIfBound(*this, *InOperation);
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