#include "PieceActor.h"

APieceActor::APieceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
}

void APieceActor::BeginPlay()
{
	Super::BeginPlay();
	
}