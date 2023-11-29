// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardManager.generated.h"

UCLASS()
class DAVID_API ABoardManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoardManager();

	void GenerateBoardSquares();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Board")
	int BoardHeight = 6;

	UPROPERTY(EditAnywhere, Category = "Board")
	int BoardWidth = 6;

	UPROPERTY(EditAnywhere, Category = "Board")
	float SquaresOffset = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Board")
	TSubclassOf<AActor> BoardSquareBP;
};
