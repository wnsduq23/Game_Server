#pragma once

#include <basetsd.h>
#include <list>
#include "NetworkManager.h"
#include "SerializePacket.h"
#include "Sector.h"


/*========================
*		DEFINE
========================*/
# define PACKET_DEBUG

// ATTACK TYPE Define
#define dfATTACK_TYPE_ATTACK1	1
#define dfATTACK_TYPE_ATTACK2	2
#define dfATTACK_TYPE_ATTACK3	3

// Init Setting
#define dfINIT_X (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) / 2
#define dfINIT_Y (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) / 2
#define dfMAX_HP 100


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
	Player(Session* pSession, int ID);
	~Player();

private:
	Session* _pSession;
	Sector* _pSector;
private:
	int		_ID;
	char		_headDirection;
	char		_moveDirection;
	short		_x;
	short		_y;
	char		_hp;
	char		_packet;
	BOOL		_bMove;

public:
	Session* GetSession() { return _pSession; }
	Sector* GetSector() { return _pSector; }
	int		GetID() { return _ID; }
	char		GetMoveDirection() { return _moveDirection; }
	char		GetHeadDirection() { return _headDirection; }
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
	void SetPlayerMoveStart(char& moveDirection, short& x, short& y);
	void SetPlayerMoveStop(char& direction, short& x, short& y);
	void SetPlayerAttack1(Player*& pDamagedPlayer, char& direction, short& x, short& y);
	void SetPlayerAttack2(Player*& pDamagedPlayer, char& direction, short& x, short& y);
	void SetPlayerAttack3(Player*& pDamagedPlayer, char& direction, short& x, short& y);
public:
	void MoveUpdate();
private:
	bool CheckMovable(short x, short y);
};
