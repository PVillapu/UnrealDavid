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

protected:
	virtual void BeginPlay() override;

protected:

	USkeletalMeshComponent* SkeletalMeshComponent;
};
