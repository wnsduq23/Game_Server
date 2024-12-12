#pragma once
#include <basetsd.h>
#include <list>
#include "NetworkManager.h"
#include "SerializeBuffer.h"


/*========================
*		DEFINE
========================*/
# define PACKET_DEBUG

// ATTACK TYPE Define
#define dfATTACK_TYPE_ATTACK1	1
#define dfATTACK_TYPE_ATTACK2	2
#define dfATTACK_TYPE_ATTACK3	3

// ATTACK RANGE Define
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

// ATTACK DAMAGE Define
#define dfATTACK1_DAMAGE		10
#define dfATTACK2_DAMAGE		20
#define dfATTACK3_DAMAGE		30

// MOVE UNIT Define
#define dfMOVE_X 3
#define dfMOVE_Y 2

// DIRECTION TYPE Define
#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7

// POSITION RANGE Define
#define dfERROR_RANGE		50
#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

// Player Packet Flag
#define dfPLAYER_PACKET_MOVE_START	0b00000001
#define dfPLAYER_PACKET_MOVE_STOP	0b00000010
#define dfPLAYER_PACKET_ATTACK1		0b00000100
#define dfPLAYER_PACKET_ATTACK2		0b00001000
#define dfPLAYER_PACKET_ATTACK3		0b00010000

// Player State Flag
#define dfPLAYER_STATE_ALIVE		0b00000001
#define dfPLAYER_STATE_MOVE			0b00000010

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


class Player
{
public:
	Player(Session* pSession, UINT32 ID);
	~Player();
private:
	Session* _pSession;

	UINT32	_ID;
	UINT8	_headDirection;
	UINT8	_moveDirection;
	UINT16	_x;
	UINT16	_y;
	UINT8	_hp;
	UINT8	_packet;
	UINT8	_state;

	SerializeBuffer _packetBuffer;

public:
	Session* GetSession() { return _pSession; }
	UINT32	GetID() { return _ID; }
	UINT8	GetMoveDirection() { return _moveDirection; }
	UINT8	GetHeadDirection() { return _headDirection; }
	UINT16	GetX() { return _x; }
	UINT16	GetY() { return _y; }
	UINT8	GetHp() { return _hp; }
	
	/*========================
	*	CLASS FUNCTION
	========================*/
public:
	void MoveUpdate();
	void TakeDamage(UINT8 damage);
	void DeqFromRecvbufANDHandlePacket();
private:
	void HandlePacketMoveStart();
	void HandlePacketMoveStop();
	void HandlePacketAttack1();
	void HandlePacketAttack2();
	void HandlePacketAttack3();
public:
	bool GetPacketMoveStart() { return (_packet & dfPLAYER_PACKET_MOVE_START); }
	bool GetPacketMoveStop() { return (_packet & dfPLAYER_PACKET_MOVE_STOP); }
	bool GetPacketAttack1() { return (_packet & dfPLAYER_PACKET_ATTACK1); }
	bool GetPacketAttack2() { return (_packet & dfPLAYER_PACKET_ATTACK2); }
	bool GetPacketAttack3() { return (_packet & dfPLAYER_PACKET_ATTACK3); }
public:
	bool GetStateAlive() { return (_state & dfPLAYER_STATE_ALIVE); }
	bool GetStateMoving() { return (_state & dfPLAYER_STATE_MOVE); }
public:
	void ResetPacketMoveStart() { _packet ^= dfPLAYER_PACKET_MOVE_START; }
	void ResetPacketMoveStop() { _packet ^= dfPLAYER_PACKET_MOVE_STOP; }
	void ResetPacketAttack1() { _packet ^= dfPLAYER_PACKET_ATTACK1; }
	void ResetPacketAttack2() { _packet ^= dfPLAYER_PACKET_ATTACK2; }
	void ResetPacketAttack3() { _packet ^= dfPLAYER_PACKET_ATTACK3; }
	// Packet Flag Setting
private:
	void SetPacketMoveStart() { _packet |= dfPLAYER_PACKET_MOVE_START; }
	void SetPacketMoveStop() { _packet |= dfPLAYER_PACKET_MOVE_STOP; }
	void SetPacketAttack1() { _packet |= dfPLAYER_PACKET_ATTACK1; }
	void SetPacketAttack2() { _packet |= dfPLAYER_PACKET_ATTACK2; }
	void SetPacketAttack3() { _packet |= dfPLAYER_PACKET_ATTACK3; }
	// State Flag Setting
private:
	void SetStateAlive() { _state |= dfPLAYER_STATE_ALIVE; }
	void SetStateDead() { _state ^= dfPLAYER_STATE_ALIVE; }
	void SetStateMoveStart() { _state |= dfPLAYER_STATE_MOVE; }
	void SetStateMoveStop() { _state ^= dfPLAYER_STATE_MOVE; }
};

class PlayerManager
{
private:
	PlayerManager();
	~PlayerManager();
	static PlayerManager* _PlayerMgr;
	std::list<Player*> _playerList;
	UINT32	_give_id_to_player = 1;
	SerializeBuffer _packetBuffer;
public:
    // 복사 및 할당 방지
    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;
public:
	/*========================
	*	CLASS FUNCTION
	========================*/
	//싱글톤 인스턴스 접근
	static PlayerManager* GetInstance()
	{
		if (_PlayerMgr == nullptr)
		{
			_PlayerMgr = new PlayerManager();
		}
		return _PlayerMgr;
	}
	void CreatePlayer(Session* newSession);
	void PlayerActionProc();
	void DestroyDeadPlayers();
	void DestroyAllPlayers();
private:
	void ChkHitBroadcast(UINT8 attackType, UINT32 ID,
		UINT8 direction, UINT16 x, UINT16 y);
	void HeaderANDMsgUnicast(int size, Player* pPlayer);
	void HeaderANDMsgBroadcast(int size, Player* pExpPlayer = nullptr);
};

