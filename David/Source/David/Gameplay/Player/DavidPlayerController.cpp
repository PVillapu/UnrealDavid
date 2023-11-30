#include "DavidPlayerController.h"
#include "Camera/CameraComponent.h"

ADavidPlayerController::ADavidPlayerController()
{
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
}

void ADavidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//InputComponent->BindAxis();
	//InputComponent->BindAction();
}

void ADavidPlayerController::Tick(float DeltaSeconds)
{
	HandleInput();
}

void ADavidPlayerController::HandleInput()
{
	ProcessGeneralInteraction();
}

void ADavidPlayerController::ProcessGeneralInteraction()
{
	/*FHitResult Hit;

	GetHitResultUnderCursor(ActionsTraceChannel, false, Hit);
	*/
	/*if (Hit.bBlockingHit && IsValid(Hit.GetActor())) 
	{
		UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *Hit.GetActor()->GetName());
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("No Actors were hit"));
	}*/
}
