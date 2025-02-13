#include "Player.h"
#include "SetSCPacket.h"
#include "Protocol.h"
#include "IngameManager.h"

#define RECV_PACKET_DEBUG

Player::Player(Session* pSession, int ID)
	: _pSession(pSession), _pSector(nullptr), _ID(ID),
	_headDirection(dfMOVE_DIR_RR),
	_moveDirection(dfMOVE_DIR_LL),
	_hp(dfMAX_HP), _bMove(false)
{
	_x = rand() % dfRANGE_MOVE_RIGHT;
	_y = rand() % dfRANGE_MOVE_BOTTOM;
}
Player::~Player()
{
}

bool Player::CheckMovable(short x, short y)
{
	if (x < dfRANGE_MOVE_LEFT || x > dfRANGE_MOVE_RIGHT ||
		y < dfRANGE_MOVE_TOP || y > dfRANGE_MOVE_BOTTOM)
		return false;

	return true;
}

void Player::MoveUpdate()
{
	switch (_moveDirection)
	{
	case dfMOVE_DIR_LL:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y))
			_x -= dfSPEED_PLAYER_X;

		if (_x < _pSector->_xPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_LL);
		break;

	case dfMOVE_DIR_LU:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y - dfSPEED_PLAYER_Y))
		{
			_x -= dfSPEED_PLAYER_X;
			_y -= dfSPEED_PLAYER_Y;
		}

		if (_x < _pSector->_xPosMin &&
			_y < _pSector->_yPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_LU);

		else if (_x < _pSector->_xPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_LL);

		else if (_y < _pSector->_yPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_UU);
		break;

	case dfMOVE_DIR_UU:
		if (CheckMovable(_x, _y - dfSPEED_PLAYER_Y))
			_y -= dfSPEED_PLAYER_Y;
		if (_y < _pSector->_yPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_UU);
		break;

	case dfMOVE_DIR_RU:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y - dfSPEED_PLAYER_Y))
		{
			_x += dfSPEED_PLAYER_X;
			_y -= dfSPEED_PLAYER_Y;
		}

		if (_x > _pSector->_xPosMax &&
			_y < _pSector->_yPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_RU);

		else if (_x > _pSector->_xPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_RR);

		else if (_y < _pSector->_yPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_UU);
		break;

	case dfMOVE_DIR_RR:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y))
			_x += dfSPEED_PLAYER_X;

		if (_x > _pSector->_xPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_RR);
		break;

	case dfMOVE_DIR_RD:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y + dfSPEED_PLAYER_Y))
		{
			_x += dfSPEED_PLAYER_X;
			_y += dfSPEED_PLAYER_Y;
		}

		if (_x > _pSector->_xPosMax &&
			_y > _pSector->_yPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_RD);

		else if (_x > _pSector->_xPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_RR);

		else if (_y > _pSector->_yPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_DD);
		break;

	case dfMOVE_DIR_DD:
		if (CheckMovable(_x, _y + dfSPEED_PLAYER_Y))
			_y += dfSPEED_PLAYER_Y;

		if (_y > _pSector->_yPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_DD);
		break;

	case dfMOVE_DIR_LD:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y + dfSPEED_PLAYER_Y))
		{
			_x -= dfSPEED_PLAYER_X;
			_y += dfSPEED_PLAYER_Y;
		}

		if (_x < _pSector->_xPosMin &&
			_y > _pSector->_yPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_LD);

		else if (_x < _pSector->_xPosMin)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_LL);

		else if (_y > _pSector->_yPosMax)
			IngameManager::GetInstance().UpdateSector(this, dfMOVE_DIR_DD);
		break;
	}
}

void Player::SetPlayerMoveStart(char& moveDirection, short& x, short& y)
{
	_x = x;
	_y = y;
	_bMove = true;
	_moveDirection = moveDirection;

	switch (moveDirection)
	{
	case  dfMOVE_DIR_LL:
	case  dfMOVE_DIR_LU:
	case  dfMOVE_DIR_LD:
		_headDirection = dfMOVE_DIR_LL;
		break;

	case  dfMOVE_DIR_RU:
	case  dfMOVE_DIR_RR:
	case  dfMOVE_DIR_RD:
		_headDirection = dfMOVE_DIR_RR;
		break;
	}	
}

void Player::SetPlayerMoveStop(char& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_bMove = false;
	_headDirection = direction;	
}

void Player::SetPlayerAttack1(Player*& pDamagedPlayer, char& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;

		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)// this라고 하면 될라나?
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK1_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_LL]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_LL]->_players.end(); iter++)
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;
		
		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK1_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_RR]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_RR]->_players.end(); iter++)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if(_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
}

void Player::SetPlayerAttack2(Player*& pDamagedPlayer, char& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;

		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK2_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_LL]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_LL]->_players.end(); iter++)
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;

		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK2_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_RR]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_RR]->_players.end(); iter++)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
}
void Player::SetPlayerAttack3(Player*& pDamagedPlayer, char& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;

		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK3_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_LL]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_LL]->_players.end(); iter++)
			{
				int dist = _x - (*iter)->_x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_LU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_LU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = _x - (*iter)->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		vector<Player*>::iterator iter;
		Sector* pSector = _pSector;

		iter = pSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; iter < pSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
		{
			if ((*iter) != this)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK3_RANGE_X)
		{
			iter = pSector->_around[dfMOVE_DIR_RR]->_players.begin();
			for (; iter < pSector->_around[dfMOVE_DIR_RR]->_players.end(); iter++)
			{
				int dist = (*iter)->_x - _x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = (*iter);
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0)
					{
						//_deadCnt++;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_RD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_RD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_UU]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_UU]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				iter = pSector->_around[dfMOVE_DIR_DD]->_players.begin();
				for (; iter < pSector->_around[dfMOVE_DIR_DD]->_players.end(); iter++)
				{
					int dist = (*iter)->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs((*iter)->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = (*iter);
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0)
						{
							//_deadCnt++;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
	}
}
