#include "NetworkManager.h"
#include "IngameManager.h"
#include "SystemLog.h"
#include "main.h"

CrashDump g_dump;

int main(void)
{
	srand(GetCurrentThreadId());
	SYSLOG_DIRECTORY(L"SystemLog");
	SYSLOG_LEVEL(SystemLog::DEBUG_LEVEL);
	LOG(L"FightGame", SystemLog::SYSTEM_LEVEL, L"%s", L"Main Thread Start\n");
	NetworkManager& networkManager = NetworkManager::GetInstance();
	IngameManager& ingameManager = IngameManager::GetInstance();

	while (1)
	{
		//network
		networkManager.NetworkUpdate();

		//Game logic
		ingameManager.GameContentsModule();
	}
	LOG(L"FightGame", SystemLog::SYSTEM_LEVEL, L"%s", L"Main Thread Terminate\n");
	return (0);
}