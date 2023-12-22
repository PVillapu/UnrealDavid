#include "CardDragDropOperation.h"

void UCardDragDropOperation::OnCardDragged(const FPointerEvent& PointerEvent)
{
	OnCardDrag.Broadcast(this, PointerEvent);
}
