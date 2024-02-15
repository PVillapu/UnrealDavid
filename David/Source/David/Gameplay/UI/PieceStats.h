#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PieceStats.generated.h"

UCLASS()
class DAVID_API UPieceStats : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowPieceStats();

private:
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UTextBlock* PieceCurrentHealthText;
	
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* PieceCurrentAttackText;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UImage* HealthBackgroundImage;

	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UImage* AttackBackgroundImage;
};
