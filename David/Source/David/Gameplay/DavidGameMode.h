#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DavidGameMode.generated.h"

UCLASS()
class DAVID_API ADavidGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADavidGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* NewPlayer) override;
};
