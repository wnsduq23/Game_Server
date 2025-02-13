#pragma once

#include "Player.h"
#include "Protocol.h"
#include "Sector.h"
#include "ObjectPool.h"
#pragma comment(lib,"Winmm.lib")


class IngameManager
{
private:
	IngameManager();
	~IngameManager();
public:
	IngameManager(const IngameManager&) = delete;
	IngameManager& operator=(const IngameManager&) = delete;
public:
	//教臂沛 按眉 积己
	static IngameManager& GetInstance()
	{
		static IngameManager _IngameMgr;
		return _IngameMgr;
	}

	ObjectPool<Player>* _pPlayerPool;
public:
	/*========================
	*		FUNCTION
	========================*/
	bool SkipForFixedFrame(void);
	void GameContentsModule();

// Sector 包访
private:
	Sector _Sectors[dfSECTOR_CNT_Y][dfSECTOR_CNT_X];
	int _sectorCnt[dfMOVE_DIR_MAX] =
	{ dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM };

public:
	void UpdateSector(Player* pPlayer, UINT16 Direction);
	void SetPlayerSector(Player* pPlayer);
	void SetSectorsAroundInfo();
	void EnqMsgOneSector(char* msg, int size, Sector* sector, Session* pExpSession = nullptr);
	void EnqMsgAroundSector(char* msg, int size, Sector* centerSector, Session* pExpSession = nullptr);

// Player 包访
	UINT32 _playerID = 0;
	Player* _Players[dfSESSION_MAX]; // Use Session ID for Index
public:
	void CreatePlayer(Session* newSession);
private:
	void PlayerActionProc();
};
