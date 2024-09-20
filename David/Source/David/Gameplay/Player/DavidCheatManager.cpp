#include "DavidCheatManager.h"
#include "DavidPlayerController.h"

void UDavidCheatManager::InitCheatManager()
{
    DavidPC = Cast<ADavidPlayerController>(GetPlayerController());
}

void UDavidCheatManager::InfiniteGold()
{
#if UE_WITH_CHEAT_MANAGER
    DavidPC->Server_InfiniteGoldCheat();
#endif
}

void UDavidCheatManager::InfiniteTurnDuration()
{
#if UE_WITH_CHEAT_MANAGER
    DavidPC->Server_InfiniteTurnTimeCheat();
#endif
}

void UDavidCheatManager::GiveCardToPlayer(int32 CardId)
{
#if UE_WITH_CHEAT_MANAGER
    DavidPC->Server_GiveCardToPlayerCheat(CardId);
#endif
}
