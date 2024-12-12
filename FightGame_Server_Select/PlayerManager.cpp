#include "PlayerManager.h"
#include "NetworkManager.h"
#include "CreateSCPacket.h"
#include "PacketDefine.h"

PlayerManager::PlayerManager()
{

}

PlayerManager::~PlayerManager()
{

}

void PlayerManager::DestroyAllPlayers()
{
	Player* player;
	while (!_playerList.empty())
	{
		player = _playerList.back();
		_playerList.pop_back();
		//player->GetSession()->SetSessionDead();
		delete(player);
	}
}

void PlayerManager::DestroyDeadPlayers()
{
	for (std::list<Player*>::iterator i = _playerList.begin(); i != _playerList.end();)
	{
		// Destroy Dead Player
		if (!(*i)->GetStateAlive())
		{
			Player* deletedPlayer = (*i);

			_packetBuffer.Clear();

			int DeleteCharSize = Create_PACKET_SC_DELETE_CHARACTER(&_packetBuffer, deletedPlayer->GetID());
			HeaderANDMsgBroadcast(DeleteCharSize);

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);

			deletedPlayer->GetSession()->SetSessionDead();
			i = _playerList.erase(i);
			delete(deletedPlayer);
			printf("죽은 플레이어가 생겼습니다!\n");
		}
		else
			i++;
	}
}

void PlayerManager::PlayerActionProc()
{
	DestroyDeadPlayers();

	for (std::list<Player*>::iterator i = _playerList.begin(); i != _playerList.end(); i++)
	{
		// recvbuf에서 header 뺀 후에 type별로 분기 + 행동
		(*i)->DeqFromRecvbufANDHandlePacket();
		// 정보에 맞게 행동으로 옮김
		(*i)->Move();

		// 옮긴 행동을 다른 클라들에게 알림 -> SC PACKETS을 broadcasting함
		if ((*i)->GetPacketMoveStart())
		{
			_packetBuffer.Clear();

			int movestart_size = Create_PACKET_SC_MOVE_START(&_packetBuffer,
				(*i)->GetID(), (*i)->GetMoveDirection(), (*i)->GetX(), (*i)->GetY());
			HeaderANDMsgBroadcast(movestart_size, (*i));
			(*i)->ResetPacketMoveStart();

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		}

		if ((*i)->GetPacketMoveStop())
		{
			_packetBuffer.Clear();

			int movestop_size = Create_PACKET_SC_MOVE_STOP(&_packetBuffer,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
			HeaderANDMsgBroadcast(movestop_size, (*i));
			(*i)->ResetPacketMoveStop();

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);

		}

		if ((*i)->GetPacketAttack1())
		{
			_packetBuffer.Clear();

			int attack1_size = Create_PACKET_SC_ATTACK1(&_packetBuffer,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
			HeaderANDMsgBroadcast(attack1_size, (*i));
			(*i)->ResetPacketAttack1();

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);

			ChkHitBroadcast(dfATTACK_TYPE_ATTACK1,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
		}

		if ((*i)->GetPacketAttack2())
		{
			_packetBuffer.Clear();

			int attack2_size = Create_PACKET_SC_ATTACK2(&_packetBuffer,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
			HeaderANDMsgBroadcast(attack2_size, (*i));
			(*i)->ResetPacketAttack2();

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);

			ChkHitBroadcast(dfATTACK_TYPE_ATTACK2,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
		}

		if ((*i)->GetPacketAttack3())
		{
			_packetBuffer.Clear();

			int attack3_size = Create_PACKET_SC_ATTACK3(&_packetBuffer,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
			HeaderANDMsgBroadcast(attack3_size, (*i));
			(*i)->ResetPacketAttack3();

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);

			ChkHitBroadcast(dfATTACK_TYPE_ATTACK3,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY());
		}
	}
}

void PlayerManager::CreatePlayer(Session* newSession)
{
	Player* player = new Player(newSession, _give_id_to_player);
	_playerList.push_back(player);

		// Send <Allocate ID Message> to New Player
	int CreateMyCharSize = Create_PACKET_SC_CREATE_MY_CHARACTER(&_packetBuffer,
		player->GetID(), player->GetHeadDirection(), player->GetX(), player->GetY(), player->GetHp());

	printf("Function %s Line %d :", __func__, __LINE__);
	HeaderANDMsgUnicast(CreateMyCharSize, player);

	// Send <Create New Player Message> to All Player
	int CreateOtherCharSize = Create_PACKET_SC_CREATE_OTHER_CHARACTER(&_packetBuffer,
		player->GetID(), player->GetHeadDirection(), player->GetX(), player->GetY(), player->GetHp());
	printf("Function %s Line %d :", __func__, __LINE__);
	HeaderANDMsgBroadcast(CreateOtherCharSize, player);

	// Send <Create All Players Message> to New Player
	for (std::list<Player*>::iterator i = _playerList.begin(); i != _playerList.end(); i++)
	{
		if ((*i)->GetID() != _give_id_to_player)
		{
			int CreateOtherCharAllSize = Create_PACKET_SC_CREATE_OTHER_CHARACTER(&_packetBuffer,
				(*i)->GetID(), (*i)->GetHeadDirection(), (*i)->GetX(), (*i)->GetY(), (*i)->GetHp());
			HeaderANDMsgUnicast(CreateOtherCharAllSize, player);
		}
	}

	_give_id_to_player++;
}

void PlayerManager::HeaderANDMsgUnicast(int size, Player* pPlayer)
{
	NetworkManager::GetInstance()->EnqSendBufUnicast(_packetBuffer.GetReadPtr(), size, pPlayer->GetSession());
	_packetBuffer.MoveReadPos(size);
}

void PlayerManager::HeaderANDMsgBroadcast(int size, Player* pExpPlayer)
{
	if (pExpPlayer != nullptr)
	{
		NetworkManager::GetInstance()->EnqSendBufBroadcast(_packetBuffer.GetReadPtr(), size, pExpPlayer->GetSession());
		_packetBuffer.MoveReadPos(size);
	}
	else
	{
		NetworkManager::GetInstance()->EnqSendBufBroadcast(_packetBuffer.GetReadPtr(), size);
		_packetBuffer.MoveReadPos(size);
	}
}

void PlayerManager::ChkHitBroadcast(UINT8 attackType, UINT32 ID, UINT8 direction, UINT16 x, UINT16 y)
{
	UINT16 rangeX;
	UINT16 rangeY;
	UINT16 damage;

	switch (attackType)
	{
	case dfATTACK_TYPE_ATTACK1:
		rangeX = dfATTACK1_RANGE_X;
		rangeY = dfATTACK1_RANGE_Y;
		damage = dfATTACK1_DAMAGE;
		break;

	case dfATTACK_TYPE_ATTACK2:
		rangeX = dfATTACK2_RANGE_X;
		rangeY = dfATTACK2_RANGE_Y;
		damage = dfATTACK2_DAMAGE;
		break;

	case dfATTACK_TYPE_ATTACK3:
		rangeX = dfATTACK3_RANGE_X;
		rangeY = dfATTACK3_RANGE_Y;
		damage = dfATTACK3_DAMAGE;
		break;

	default:
		return;
	}

	UINT16 minX, maxX, minY, maxY;

    if (direction == dfPACKET_MOVE_DIR_LL)
    {
        minX = x - rangeX;
        maxX = x;
    }
    else if (direction == dfPACKET_MOVE_DIR_RR)
    {
        minX = x;
        maxX = x + rangeX;
    }
    else
    {
        return; // Invalid direction
    }

    minY = y - rangeY;
    maxY = y + rangeY;

	for (std::list<Player*>::iterator i = _playerList.begin(); i != _playerList.end(); i++)
	{
		Player* player = *i;

		if (player->GetX() >= minX && player->GetX() <= maxX &&
			player->GetY() >= minY && player->GetY() <= maxY &&
			ID != player->GetID())
		{
			_packetBuffer.Clear();

			player->TakeDamage(damage);
			int damage_size = Create_PACKET_SC_DAMAGE(&_packetBuffer, ID, (*i)->GetID(), (*i)->GetHp());
			HeaderANDMsgBroadcast(damage_size);

			if (_packetBuffer.GetReadPtr() != _packetBuffer.GetWritePtr())
				printf("Packet Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
		}
	}
}
