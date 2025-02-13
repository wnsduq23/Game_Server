#include "Sector.h"
#include "SetSCPacket.h"


Sector::Sector()
{

}

Sector::~Sector()
{

}

void Sector::InitializeSector(short xIndex, short yIndex)
{
	_xIndex = xIndex; // ±ª¿Ã?
	_yIndex = yIndex;

	if (_xIndex < 2 || _xIndex >= (dfSECTOR_CNT_X - 2) ||
		_yIndex < 2 || _yIndex >= (dfSECTOR_CNT_Y - 2))
		return;

	_xPosMin = (xIndex - 2) * dfSECTOR_SIZE_X;
	_yPosMin = (yIndex - 2) * dfSECTOR_SIZE_Y;
	_xPosMax = (xIndex - 1) * dfSECTOR_SIZE_X;
	_yPosMax = (yIndex - 1) * dfSECTOR_SIZE_Y;
	_players.reserve(dfDEFAULT_PLAYERS_PER_SECTOR); // ø÷ 1024?
}
