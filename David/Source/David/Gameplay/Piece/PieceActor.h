#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PieceActor.generated.h"

UCLASS()
class DAVID_API APieceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APieceActor();

	virtual void SetupPiece(class ABoardManager* BoardManager);

	virtual void ProcessTurn();

	virtual void ProcessAction(int32 ActionID);

protected:
	virtual void BeginPlay() override;

protected:

	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY()
	ABoardManager* Board;
};
