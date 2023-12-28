#include "DavidPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "../Board/BoardManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"

bool ADavidPlayerController::GetBoardHitUnderCursor(FHitResult& Hit, const FVector2D MousePosition)
{
	FIntPoint MousePos;
	GEngine->GameViewport->Viewport->GetMousePos(MousePos);
	FVector2D MousePosV = FVector2D(MousePos.X, MousePos.Y);

	UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *MousePosV.ToString());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector WorldLocation, WorldDirection;
	UGameplayStatics::DeprojectScreenToWorld(this, MousePosV, WorldLocation, WorldDirection);
	
	FVector CameraLocation = PlayerCameraManager->GetCameraLocation();

	GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, (WorldLocation - CameraLocation) * 100000, BoardCollisionChannel, QueryParams);
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 10.f, 0U, 1.f);

	if (Hit.bBlockingHit && IsValid(Hit.GetActor()))
	{
		UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *Hit.GetActor()->GetName());
		DrawDebugLine(GetWorld(), CameraLocation, (WorldLocation - CameraLocation) * 100000, FColor::Blue, false, 10.f, 0U, 1.f);
		return true;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("No Actors were hit"));
		return false;
	}
}

void ADavidPlayerController::Client_SetDavidPlayerIndex_Implementation(int32 PIndex)
{
	PlayerIndex = PIndex;

	if (BoardManager == nullptr) 
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
		if (OutActors.Num() > 0)
		{
			BoardManager = Cast<ABoardManager>(OutActors[0]);
		}
	}

	PlayerCameraActor = BoardManager->GetPlayerCameraActor(PlayerIndex);
	UActorComponent* CameraSpot = PlayerCameraActor->GetComponentByClass(UCameraComponent::StaticClass());

	if (CameraSpot == nullptr) return;

	PlayerCamera = Cast<UCameraComponent>(CameraSpot);

	if (PlayerCameraActor)
	{
		SetViewTarget(PlayerCameraActor);
	}
}

void ADavidPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent)) 
	{
		EnhancedInputComponent->BindAction(PlayCardAction, ETriggerEvent::Triggered, this, &ADavidPlayerController::OnPlayedCardAction);
	}
}

void ADavidPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PlayerHUD = CreateWidget<UUserWidget>(GetGameInstance(), PlayerHUDClass);

	if (PlayerHUD != nullptr) 
	{
		PlayerHUD->AddToViewport();
	}

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;

	if (BoardManager == nullptr) 
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoardManager::StaticClass(), OutActors);
		if (OutActors.Num() > 0)
		{
			BoardManager = Cast<ABoardManager>(OutActors[0]);
		}
	}

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}
}

void ADavidPlayerController::ProcessGeneralInteraction()
{
	/*FHitResult Hit;

	GetHitResultUnderCursor(ActionsTraceChannel, false, Hit);
	
	if (Hit.bBlockingHit && IsValid(Hit.GetActor())) 
	{
		UE_LOG(LogTemp, Log, TEXT("Trace hit actor: %s"), *Hit.GetActor()->GetName());
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("No Actors were hit"));
	}*/
}

void ADavidPlayerController::OnPlayedCardAction()
{
	UE_LOG(LogTemp, Log, TEXT("Holi"));
}
