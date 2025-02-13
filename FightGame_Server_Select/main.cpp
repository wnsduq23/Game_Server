#include "main.h"
#include "NetworkManager.h"
#include "IngameManager.h"

bool g_bShutdown = false;

int main(void)
{
	NetworkManager& networkManager = NetworkManager::GetInstance();
	IngameManager& ingameManager = IngameManager::GetInstance();

	while (!g_bShutdown)
	{
		//network
		networkManager.NetworkUpdate();

		//Game logic
		ingameManager.GameContentsModule();
	}
	return (0);
}