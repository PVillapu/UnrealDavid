#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "CardDragDropOperation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardDragDropMulticast, UDragDropOperation*, Operation, const FPointerEvent&, PointerEvent);

UCLASS()
class DAVID_API UCardDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnCardDragDropMulticast OnCardDrag;

private:
	UFUNCTION(BlueprintCallable)
	void OnCardDragged(const FPointerEvent& PointerEvent);

};
