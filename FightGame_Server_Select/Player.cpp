#include "PlayerManager.h"
#include "CreateSCPacket.h"
#include "PacketDefine.h"

#define RECV_PACKET_DEBUG

Player::Player(Session* pSession, UINT32 ID)
	: _pSession(pSession), _ID(ID),
	_headDirection(dfPACKET_MOVE_DIR_RR),
	_moveDirection(dfPACKET_MOVE_DIR_LL),
	_x(dfINIT_X), _y(dfINIT_Y), _hp(dfMAX_HP),
	_packet(0), _state(dfPLAYER_STATE_ALIVE)
{

}
Player::~Player()
{
	SetStateDead();
}

void Player::Move()
{
	if (_x < dfRANGE_MOVE_LEFT || _x > dfRANGE_MOVE_RIGHT ||
		_y > dfRANGE_MOVE_BOTTOM || _y < dfRANGE_MOVE_TOP)
	{

	}
	else if (GetStateMoving())
	{
		switch (_moveDirection)
		{
		case dfPACKET_MOVE_DIR_LL:
			_x -= dfMOVE_X;
			break;

		case dfPACKET_MOVE_DIR_LU:
			_x -= dfMOVE_X;
			_y -= dfMOVE_Y;
			break;

		case dfPACKET_MOVE_DIR_UU:
			_y -= dfMOVE_Y;
			break;

		case dfPACKET_MOVE_DIR_RU:
			_x += dfMOVE_X;
			_y -= dfMOVE_Y;
			break;

		case dfPACKET_MOVE_DIR_RR:
			_x += dfMOVE_X;
			break;

		case dfPACKET_MOVE_DIR_RD:
			_x += dfMOVE_X;
			_y += dfMOVE_Y;
			break;

		case dfPACKET_MOVE_DIR_DD:
			_y += dfMOVE_Y;
			break;

		case dfPACKET_MOVE_DIR_LD:
			_x -= dfMOVE_X;
			_y += dfMOVE_Y;
			break;
		}
	}
}

void Player::TakeDamage(UINT8 damage)
{
	_hp -= damage;
	if (_hp <= 0 || _hp > 100)
	{
		_hp = 0;
		SetStateDead();
	}
}

void Player::DeqFromRecvbufANDHandlePacket()
{
		int iUsedSize = _pSession->_recvBuf.GetUseSize();
	while (iUsedSize > 0)
	{
		if (iUsedSize <= dfPACKET_HEADER_SIZE)
			break;

		stPACKET_HEADER header;
		int peekRet = _pSession->_recvBuf.Peek((char*)&header, dfPACKET_HEADER_SIZE);
		if (peekRet != dfPACKET_HEADER_SIZE)
		{
			printf("Error! Func %s Line %d\n", __func__, __LINE__);
			SetStateDead();
			return;
		}

		if (header.code != dfPACKET_HEADER_CODE)
		{
			printf("Error! Wrong Header Code! - Func %s Line %d\n", __func__, __LINE__);
			SetStateDead();
			return;
		}

		if (iUsedSize < dfPACKET_HEADER_SIZE + header.payload_size)
			break;

		int moveReadRet = _pSession->_recvBuf.MoveReadPos(dfPACKET_HEADER_SIZE);
		if (moveReadRet != dfPACKET_HEADER_SIZE)
		{
			printf("Error! Func %s Line %d\n", __func__, __LINE__);
			SetStateDead();
			return;
		}

		_packetBuffer.Clear();

		switch (header.action_type)
		{
		case dfPACKET_CS_MOVE_START:
			HandlePacketMoveStart();
			break;

		case dfPACKET_CS_MOVE_STOP:
			HandlePacketMoveStop();
			break;

		case dfPACKET_CS_ATTACK1:
			HandlePacketAttack1();
			break;

		case dfPACKET_CS_ATTACK2:
			HandlePacketAttack2();
			break;

		case dfPACKET_CS_ATTACK3:
			HandlePacketAttack3();
			break;
		}

		if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
			printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		
		iUsedSize = _pSession->_recvBuf.GetUseSize();
	}
}

void Player::HandlePacketMoveStart()
{
	char moveDirection;
	short X;
	short Y;

	int size = sizeof(moveDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = _pSession->_recvBuf.Dequeue(_packetBuffer.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		SetStateDead();
		return;
	}
	_packetBuffer.MoveWritePos(size);

	_packetBuffer >> moveDirection;
	_packetBuffer >> X;
	_packetBuffer >> Y;

	if (abs(X - _x) > dfERROR_RANGE ||
		abs(Y - _y) > dfERROR_RANGE)
	{
		SetStateDead();
		return;
	}

	_moveDirection = moveDirection;
	switch (moveDirection)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		_headDirection = dfPACKET_MOVE_DIR_LL;
		break;

	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		_headDirection = dfPACKET_MOVE_DIR_RR;
		break;
	}

	_x = X;
	_y = Y;
#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: MOVE START\n\n\
packetMoveStart.headDirection: %d\n\
packetMoveStart.X: %d\n\
packetMoveStart.Y: %d\n\n\
now moveDirection: %d\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
_ID, _headDirection, X, Y, _moveDirection, _x, _y);
#endif

	SetPacketMoveStart();
	SetStateMoveStart();
}

void Player::HandlePacketMoveStop()
{
	char headDirection;
	short X;
	short Y;

	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = _pSession->_recvBuf.Dequeue(_packetBuffer.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		SetStateDead();
		return;
	}
	_packetBuffer.MoveWritePos(size);

	_packetBuffer >> headDirection;
	_packetBuffer >> X;
	_packetBuffer >> Y;

	if (abs(X - _x) > dfERROR_RANGE ||
		abs(Y - _y) > dfERROR_RANGE)
	{
		SetStateDead();
		return;
	}

	_headDirection = headDirection;
	_x = X;
	_y = Y;

#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: MOVE STOP\n\n\
packetMoveStop.headDirection: %d\n\
packetMoveStop.X: %d\n\
packetMoveStop.Y: %d\n\n\
now moveDirection: %d\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
_ID, _headDirection, X, Y, _moveDirection, _x, _y);
#endif

	SetPacketMoveStop();
	SetStateMoveStop();
}

void Player::HandlePacketAttack1()
{
	char headDirection;
	short X;
	short Y;

	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = _pSession->_recvBuf.Dequeue(_packetBuffer.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		SetStateDead();
		return;
	}
	_packetBuffer.MoveWritePos(size);

	_packetBuffer >> headDirection;
	_packetBuffer >> X;
	_packetBuffer >> Y;


	if (abs(X - _x) > dfERROR_RANGE ||
		abs(Y - _y) > dfERROR_RANGE)
	{
		SetStateDead();
		return;
	}

	_headDirection = headDirection;
	_x = X;
	_y = Y;
#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 1\n\n\
packetAttack1.headDirection: %d\n\
packetAttack1.X: %d\n\
packetAttack1.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
_ID, headDirection, X, Y, _x, _y);
#endif
	SetPacketAttack1();
}

void Player::HandlePacketAttack2()
{
	char headDirection;
	short X;
	short Y;

	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = _pSession->_recvBuf.Dequeue(_packetBuffer.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		SetStateDead();
		return;
	}
	_packetBuffer.MoveWritePos(size);

	_packetBuffer >> headDirection;
	_packetBuffer >> X;
	_packetBuffer >> Y;

	if (abs(X - _x) > dfERROR_RANGE ||
		abs(Y - _y) > dfERROR_RANGE)
	{
		SetStateDead();
		return;
	}

	_headDirection = headDirection;
	_x = X;
	_y = Y;

#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 2\n\n\
packetAttack2.headDirection: %d\n\
packetAttack2.X: %d\n\
packetAttack2.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
_ID, headDirection, X, Y, _x, _y);
#endif
	SetPacketAttack2();
}

void Player::HandlePacketAttack3()
{
	char headDirection;
	short X;
	short Y;

	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = _pSession->_recvBuf.Dequeue(_packetBuffer.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		SetStateDead();
		return;
	}
	_packetBuffer.MoveWritePos(size);

	_packetBuffer >> headDirection;
	_packetBuffer >> X;
	_packetBuffer >> Y;

	if (abs(X - _x) > dfERROR_RANGE ||
		abs(Y - _y) > dfERROR_RANGE)
	{
		SetStateDead();
		return;
	}

	_headDirection = headDirection;
	_x = X;
	_y = Y;
#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 3\n\n\
packetAttack3.headDirection: %d\n\
packetAttack3.X: %d\n\
packetAttack3.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
_ID, headDirection, X, Y, _x, _y);
#endif

	SetPacketAttack3();
}