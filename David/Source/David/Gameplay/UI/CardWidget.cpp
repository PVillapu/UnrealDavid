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

//void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
//{
//	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
//
//	this->SetVisibility(ESlateVisibility::HitTestInvisible);
//	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
//
//	/*DragDropOperation->WidgetReference = this;
//	DragDropOperation->DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()); // TODO: This class is no necessary I believe
//	*/
//	
//	FVector2D abs = InGeometry.GetAbsolutePosition();
//	UE_LOG(LogTemp, Error, TEXT("Geometry abs pos: %s"), *abs.ToString());
//
//	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
//	UE_LOG(LogTemp, Error, TEXT("Cursor screen pos: %s"), *ScreenCursorPos.ToString());
//	
//	UE_LOG(LogTemp, Error, TEXT("Difference: %s"), *(abs - ScreenCursorPos).ToString());
//	
//	UCardWidget* CopyCard = CreateWidget<UCardWidget>(this->GetOwningPlayer(), UCardWidget::StaticClass());
//	CopyCard->SetVisibility(ESlateVisibility::Visible);
//	CopyCard->AddToViewport();
//
//	DragDropOperation->DefaultDragVisual = CopyCard;
//	DragDropOperation->Payload = this;
//	DragDropOperation->Pivot = EDragPivot::MouseDown;
//
//	OutOperation = DragDropOperation;
//
//	OnGrabbedCardDelegate.ExecuteIfBound(*this);
//}
//
//void UCardWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
//{
//	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
//
//	this->SetVisibility(ESlateVisibility::Visible);
//	OnLeftCardDelegate.ExecuteIfBound(*this);
//}

//FReply UCardWidget::CustomDetectDrag(const FPointerEvent& InMouseEvent, UWidget* WidgetDetectingDrag, FKey DragKey)
//{
//	if (InMouseEvent.GetEffectingButton() == DragKey /*|| PointerEvent.IsTouchEvent()*/)
//	{
//		FEventReply Reply;
//		Reply.NativeReply = FReply::Handled();
//
//		if (WidgetDetectingDrag)
//		{
//			TSharedPtr<SWidget> SlateWidgetDetectingDrag = WidgetDetectingDrag->GetCachedWidget();
//			if (SlateWidgetDetectingDrag.IsValid())
//			{
//				Reply.NativeReply = Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), DragKey);
//				return Reply.NativeReply;
//			}
//		}
//	}
//
//	return FReply::Unhandled();
//}

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