#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/Enums.h"
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

	void OnPlayerReady(EDavidPlayer Player);

protected:
	void StartGame();

private:
	UPROPERTY(Transient, SkipSerialization)
	class ABoardManager* BoardManager;

	UPROPERTY(EditAnywhere, Category = "Board")
	TSubclassOf<class ABoardManager> GameBoardClass;

	UPROPERTY(Transient, SkipSerialization)
	class ADavidPlayerController* Player1;

	UPROPERTY(Transient, SkipSerialization)
	class ADavidPlayerController* Player2;

	UPROPERTY(Transient, SkipSerialization)
	bool MatchStarted = false;

	UPROPERTY(Transient, SkipSerialization)
	bool Player1Ready = false;

	UPROPERTY(Transient, SkipSerialization)
	bool Player2Ready = false;
};
