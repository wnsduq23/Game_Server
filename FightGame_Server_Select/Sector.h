#pragma once

#include "Protocol.h"
#include <vector>

using namespace std;
class Player;

class Sector
{
public:
	Sector();
	~Sector();
	void InitializeSector(short xIndex, short yIndex);
public:
	short _xIndex;
	short _yIndex;
	short _xPosMin;
	short _yPosMin;
	short _xPosMax;
	short _yPosMax;
	std::vector<Player*> _players;

public:
	Sector* _llNew[dfVERT_SECTOR_NUM];
	Sector* _luNew[dfDIAG_SECTOR_NUM];
	Sector* _uuNew[dfVERT_SECTOR_NUM];
	Sector* _ruNew[dfDIAG_SECTOR_NUM];
	Sector* _rrNew[dfVERT_SECTOR_NUM];
	Sector* _rdNew[dfDIAG_SECTOR_NUM];
	Sector* _ddNew[dfVERT_SECTOR_NUM];
	Sector* _ldNew[dfDIAG_SECTOR_NUM];

	Sector* _llOld[dfVERT_SECTOR_NUM];
	Sector* _luOld[dfDIAG_SECTOR_NUM];
	Sector* _uuOld[dfVERT_SECTOR_NUM];
	Sector* _ruOld[dfDIAG_SECTOR_NUM];
	Sector* _rrOld[dfVERT_SECTOR_NUM];
	Sector* _rdOld[dfDIAG_SECTOR_NUM];
	Sector* _ddOld[dfVERT_SECTOR_NUM];
	Sector* _ldOld[dfDIAG_SECTOR_NUM];

public:
	Sector* _around[dfAROUND_SECTOR_NUM];
	Sector** _new[dfMOVE_DIR_MAX] =
	{
		_llNew, _luNew, _uuNew, _ruNew,
		_rrNew, _rdNew, _ddNew, _ldNew
	};

	Sector** _old[dfMOVE_DIR_MAX] =
	{
		_llOld, _luOld, _uuOld, _ruOld,
		_rrOld, _rdOld, _ddOld, _ldOld
	};
};
