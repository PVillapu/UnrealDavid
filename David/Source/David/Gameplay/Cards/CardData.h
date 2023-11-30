// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardData.generated.h"

UCLASS()
class DAVID_API UCardData : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
    UStaticMesh* CardMesh;

   /* UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
    TSubclassOf<class APiece> CardBehaviorClass;*/

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
    UTexture2D* CardImage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card Data")
    FText CardDescription;
};
