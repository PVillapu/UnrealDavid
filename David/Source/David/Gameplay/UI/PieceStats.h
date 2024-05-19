#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PieceStats.generated.h"

UCLASS()
class DAVID_API UPieceStats : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthValue(int32 Health);

	void SetAttackValue(int32 Attack);

private:
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	class UTextBlock* AttackText;
	
	UPROPERTY(EditAnywhere, Category = "David", meta = (BindWidget))
	UTextBlock* HealthText;
};
