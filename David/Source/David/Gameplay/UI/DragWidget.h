#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DragWidget.generated.h"

UCLASS()
class DAVID_API UDragWidget : public UDragDropOperation	// TODO: DELETE THIS
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UUserWidget* WidgetReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DragOffset;
};
