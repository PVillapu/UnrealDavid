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

	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* NewPlayer) override;

protected:

	UPROPERTY(Transient, SkipSerialization)
	class ABoardManager* BoardManager;

	UPROPERTY(EditAnywhere, Category = "Board")
	TSubclassOf<class ABoardManager> GameBoardClass;
};
