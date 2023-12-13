#include "DavidPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"

ADavidPlayerController::ADavidPlayerController()
{
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
}

void ADavidPlayerController::Tick(float DeltaSeconds)
{
	HandleInput();
}

void ADavidPlayerController::BeginPlay()
{
	PlayerHUD = CreateWidget<UUserWidget>(GetGameInstance(), PlayerHUDClass);

	if (PlayerHUD != nullptr) 
	{
		PlayerHUD->AddToViewport();
	}
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

//void ADavidPlayerController::PlayCardAction(const FInputActionValue& Value)
//{
//	
//}
