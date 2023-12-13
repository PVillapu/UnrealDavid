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

	virtual void SetupPiece(class ABoardManager* BoardManager, int32 PieceId);

protected:
	virtual void BeginPlay() override;

protected:

	UStaticMeshComponent* StaticMeshComponent;
	ABoardManager* Board;
	int32 PieceID;
};
