#pragma once

#include <basetsd.h>
#include <list>
#include "NetworkManager.h"
#include "SerializePacket.h"
#include "Sector.h"


/*========================
*		DEFINE
========================*/


/*========================
*		FUNCTION
========================*/


/*========================
*		CLASS
========================*/

class Sector;
class Session;

class Player
{
public:
	Player()=default;
	Player(Session* pSession, DWORD ID);
	~Player();

private:
	Session* _pSession;
	Sector* _pSector;
private:
	DWORD		_ID;
	BYTE		_headDirection;
	BYTE		_moveDirection;
	short		_x;
	short		_y;
	char		_hp;
	BOOL		_bMove;

public:
	Session* GetSession() { return _pSession; }
	Sector* GetSector() { return _pSector; }
	DWORD		GetID() { return _ID; }
	BYTE		GetMoveDirection() { return _moveDirection; }
	BYTE		GetHeadDirection() { return _headDirection; }
	short		GetX() { return _x; }
	short		GetY() { return _y; }
	char		GetHp() { return _hp; }
	bool		GetStateMoving() { return _bMove; }
	void		SetSector(Sector* pSector) { _pSector = pSector; }
	
	/*========================
	*	CLASS FUNCTION
	========================*/
public:
	// Set Game Data from Packet Data
	void SetPlayerMoveStart(BYTE& moveDirection, short& x, short& y);
	void SetPlayerMoveStop(BYTE& direction, short& x, short& y);
	void SetPlayerAttack1(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y);
	void SetPlayerAttack2(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y);
	void SetPlayerAttack3(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y);

public:
	void MoveUpdate();
private:
	bool CheckMovable(short x, short y);
};
