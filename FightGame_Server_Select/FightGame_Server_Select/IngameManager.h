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
	//�̱��� ��ü ����
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
	void GameContentsModule();

// Sector ����
private:
	Sector _Sectors[dfSECTOR_CNT_Y][dfSECTOR_CNT_X];
	int _sectorCnt[dfMOVE_DIR_MAX] =
	{ dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM,
	  dfVERT_SECTOR_NUM, dfDIAG_SECTOR_NUM };

public:
	void UpdateSector(Player* pPlayer);
	inline void SetPlayerSector(Player* pPlayer);
	inline void SetSectorsAroundInfo();
	inline void SendPacketOneSector(char* msg, int size, Sector* sector, Session* pExpSession = nullptr);
	void SendPacketAroundSector(char* msg, int size, Sector* centerSector, Session* pExpSession = nullptr);
	inline bool FixedUpdate();

// Player ����
	DWORD _playerID = 0;
	Player* _Players[dfSESSION_MAX]; // Use Session ID for Index
public:
	void CreatePlayer(Session* newSession);
private:
	inline void PlayerActionProc();
};
