#include "main.h"
#include "NetworkManager.h"
#include "PlayerManager.h"
#include "GameContents.h"

bool g_bShutdown = false;
NetworkManager* NetworkManager::_NetworkMgr = nullptr;
PlayerManager* PlayerManager::_PlayerMgr = nullptr;

int main(void)
{
	// network init
	NetworkManager::GetInstance()->NetworkInitialize();

	while (!g_bShutdown)
	{
		//network
		NetworkManager::GetInstance()->SelectModel();

		//Game logic
		GameContentsModule();
	}
	// terminate
	// 후에 db에 저장할 데이터나 마무리 할 일들이 다 끝났는 지 확인한 후 종료
	PlayerManager::GetInstance()->DestroyAllPlayers();
	NetworkManager::GetInstance()->Terminate();
	return (0);
}