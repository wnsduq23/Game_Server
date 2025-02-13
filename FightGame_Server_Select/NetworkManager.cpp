#include "NetworkManager.h"
#include "IngameManager.h"
#include "main.h"
#include <stdio.h>
#include "SetSCPacket.h"

# define NETWORK_DEBUG
//# define RECV_PACKET_DEBUG

NetworkManager::NetworkManager()
{
	_pSessionPool = new ObjectPool<Session>(dfSESSION_MAX, false);

	int err;
	int bindRet;
	int listenRet;
	int ioctRet;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		g_bShutdown = true;
		return;
	}

	// Create Socket
	_listensock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listensock == INVALID_SOCKET)
	{
		err = WSAGetLastError();
		printf("Error! Function %s Line %d: %d\n", __func__, __LINE__, err);
		g_bShutdown = true;
		return;
	}

	// Bind
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	InetPton(AF_INET, IP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT);

	bindRet = bind(_listensock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (bindRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		printf("Error! Function %s Line %d: %d\n", __func__, __LINE__, err);
		g_bShutdown = true;
		return;
	}

	// Listen
	listenRet = listen(_listensock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		printf("Error! Function %s Line %d: %d\n", __func__, __LINE__, err);
		g_bShutdown = true;
		return;
	}

	// Set Non-Blocking Mode
	u_long on = 1;
	ioctRet = ioctlsocket(_listensock, FIONBIO, &on);
	if (ioctRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		printf("Error! Function %s Line %d: %d\n", __func__, __LINE__, err);
		g_bShutdown = true;
		return;
	}

	printf("Setting Complete!\n");
}

NetworkManager::~NetworkManager()
{
	for (int i = 0; i < dfSESSION_MAX; i++)
	{
		if (_Sessions[i] != nullptr)
		{
			closesocket(_Sessions[i]->_socket);
			_pSessionPool->Free(_Sessions[i]);
		}
	}

	closesocket(_listensock);
	WSACleanup();

	delete _pSessionPool;
}

void NetworkManager::NetworkUpdate()
{ 
	//PRO_BEGIN(L"Network");

	int rStopIdx = 0;
	int wStopIdx = 0;
	int rStartIdx = 0;
	int wStartIdx = 0;

	for (int i = 0; i < dfSESSION_MAX; i++)
	{
		if (_Sessions[i] == nullptr) continue;

		_rSessions[rStopIdx++] = _Sessions[i];
		if (_Sessions[i]->_sendRingBuf.GetUseSize() > 0)
			_wSessions[wStopIdx++] = _Sessions[i];
	}

	while ((rStopIdx - rStartIdx) >= (FD_SETSIZE - 1) &&
		(wStopIdx - wStartIdx) >= FD_SETSIZE)
	{
		SelectModel(rStartIdx, (FD_SETSIZE - 1), wStartIdx, FD_SETSIZE);
		rStartIdx += (FD_SETSIZE - 1);
		wStartIdx += FD_SETSIZE;
	}

	while((rStopIdx - rStartIdx) >= (FD_SETSIZE - 1) && 
		(wStopIdx - wStartIdx) < FD_SETSIZE)
	{
		SelectModel(rStartIdx, (FD_SETSIZE - 1), 0, 0);
		rStartIdx += (FD_SETSIZE - 1);
	}

	while ((rStopIdx - rStartIdx) < (FD_SETSIZE - 1) &&
		(wStopIdx - wStartIdx) >= FD_SETSIZE)
	{
		SelectModel(0, 0, wStartIdx, FD_SETSIZE);
		wStartIdx += FD_SETSIZE;
	}

	SelectModel(rStartIdx, rStopIdx - rStartIdx, wStartIdx, wStopIdx - wStartIdx);

	//PRO_END(L"Network");

	//PRO_BEGIN(L"Delayed Disconnect");
	DisconnectDeadSessions();
	//PRO_END(L"Delayed Disconnect");	
}

void NetworkManager::SelectModel(int rStartIdx, int rCount, int wStartIdx, int wCount)
{
		FD_ZERO(&_rset);
	FD_ZERO(&_wset);

	FD_SET(_listensock, &_rset);
	for (int i = 0; i < wCount; i++)
		FD_SET(_wSessions[wStartIdx + i]->_socket, &_wset);
	for (int i = 0; i < rCount; i++)
		FD_SET(_rSessions[rStartIdx + i]->_socket, &_rset);

	// Select Socket Set
	timeval time;
	time.tv_sec = 0;
	time.tv_usec = 0;
	int selectRet = select(0, &_rset, &_wset, NULL, &time);
	if (selectRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		
		/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
			L"%s[%d]: select Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: select Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		dump.Crash();*/
		return;
	}

	// Handle Selected Socket
	else if (selectRet > 0)
	{
		if (FD_ISSET(_listensock, &_rset))
			AcceptProc();

		for (int i = 0; i < wCount; i++)
			if (FD_ISSET(_wSessions[wStartIdx + i]->_socket, &_wset))
				SendProc(_wSessions[wStartIdx + i]);

		for (int i = 0; i < rCount; i++)
			if (FD_ISSET(_rSessions[rStartIdx + i]->_socket, &_rset))
				RecvProc(_rSessions[rStartIdx + i]);
	}
}

void NetworkManager::SendProc(Session* session)
{
	if (session->_sendRingBuf.GetUseSize() <= 0)
		return;

	int sendRet = 0;
	if (session->_sendRingBuf.DirectDequeueSize() == 0)
	{
		sendRet = send(session->_socket,
			session->_sendRingBuf.GetReadPtr(),
			session->_sendRingBuf.GetUseSize(), 0);
	}
	else
	{
		sendRet = send(session->_socket,
			session->_sendRingBuf.GetReadPtr(),
			session->_sendRingBuf.DirectDequeueSize(), 0);
	}

	if (sendRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			printf("Error! Func %s Line %d: %d\n", __func__, __LINE__, err);
			session->SetSessionDead();
		}
		return;
	}

#ifdef NETWORK_DEBUG
	// For Test =======================================================
	char testBuf[DEFAULT_BUF_SIZE];
	int testPeekRet = session->_sendRingBuf.Peek(testBuf, sendRet);
	testBuf[testPeekRet] = '\0';
	printf("\n\n");
	printf("Send Test===========================\n\n");
	int idx = 0;
	session->_sendRingBuf.GetBufferDataForDebug();
	while (idx < testPeekRet)
	{
		printf("0x%X ", testBuf[idx]);
		idx++;
	}
	printf("\n\n===================================");
	printf("\n\n");

	if (testPeekRet != sendRet)
	{
		printf("Test Error! Func %s Line %d\n", __func__, __LINE__);
	}
	// ===================================================================
#endif

	int moveRet = session->_sendRingBuf.MoveReadPos(sendRet);
	if (sendRet != moveRet)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		session->SetSessionDead();
		return;
	}

}

void NetworkManager::AcceptProc()
{
	// Session Num is more than SESSION_MAX
	if (_usableCnt == 0 && _sessionIDs == dfSESSION_MAX)
	{
		/*LOG(L"FightGame", CSystemLog::DEBUG_LEVEL,
			L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);*/

		return;
	}

	int ID;
	if (_usableCnt == 0)
		ID = _sessionIDs++;
	else
		ID = _usableSessionID[--_usableCnt];

	Session* pSession = _pSessionPool->Alloc();
	pSession->Initialize(ID);

	if (pSession == nullptr)
	{
		/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
			L"%s[%d]: new Error, %d\n",
			_T(__FUNCTION__), __LINE__, ID);

		::wprintf(L"%s[%d]: new Error, %d\n",
			_T(__FUNCTION__), __LINE__, ID);

		dump.Crash();	*/
		return;
	}

	int addrlen = sizeof(pSession->_addr);
	pSession->_socket = accept(_listensock, (SOCKADDR*)&pSession->_addr, &addrlen);
	if (pSession->_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		
		/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
			L"%s[%d]: accept Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: accept Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		dump.Crash();*/
		return;
	}

	pSession->_lastRecvTime = GetTickCount64();
	_Sessions[pSession->_ID] = pSession;
	IngameManager::GetInstance().CreatePlayer(pSession);

}

void NetworkManager::RecvProc(Session* session)
{
	session->_lastRecvTime = GetTickCount64();
	int recvRet = 0;
	
	if (session->_recvRingBuf.DirectEnqueueSize() != 0)
	{
		recvRet = recv(session->_socket,
			session->_recvRingBuf.GetWritePtr(),
			session->_recvRingBuf.DirectEnqueueSize(), 0);
	}
	else if (session->_recvRingBuf.DirectEnqueueSize() == 0 &&
		session->_recvRingBuf.GetFreeSize() != 0)
	{
		recvRet = recv(session->_socket,
			session->_recvRingBuf.GetWritePtr(),
			session->_recvRingBuf.GetFreeSize(), 0);
	}
	else
	{
		session->_recvRingBuf.Resize(session->_recvRingBuf.GetBufferSize() * 1.5f);
		recvRet = recv(session->_socket,
			session->_recvRingBuf.GetWritePtr(),
			session->_recvRingBuf.DirectEnqueueSize(), 0);
	}

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			printf("Error! Func %s Line %d: %d\n", __func__, __LINE__, err);
			session->SetSessionDead();
			return;
		}
	}
	else if (recvRet == 0)
	{
		session->SetSessionDead();
		return;
	}

	int moveRet = session->_recvRingBuf.MoveWritePos(recvRet);
	if (recvRet != moveRet)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		session->SetSessionDead();
		return;
	}

#ifdef NETWORK_DEBUG
	// For Test =======================================================
	char testBuf[DEFAULT_BUF_SIZE];
	int testPeekRet = session->_recvRingBuf.Peek(testBuf, moveRet);
	testBuf[testPeekRet] = '\0';
	printf("\n\n");
	printf("Recv Test===========================\n\n");
	int idx = 0;
	while (idx < testPeekRet)
	{
		printf("0x%X ", testBuf[idx]);
		idx++;
	}
	printf("\n\n===================================");
	printf("\n\n");

	if (testPeekRet != moveRet)
	{
		printf("Test Error! Func %s Line %d\n", __func__, __LINE__);
	}
	// ===================================================================
#endif

	Player* pPlayer = IngameManager::GetInstance()._Players[session->_ID];
	int iUsedSize = session->_recvRingBuf.GetUseSize();
	while (iUsedSize > 0)
	{
		if (iUsedSize <= dfPACKET_HEADER_SIZE)
			break;

		stPACKET_HEADER header;
		int peekRet = session->_recvRingBuf.Peek((char*)&header, dfPACKET_HEADER_SIZE);
		if (peekRet != dfPACKET_HEADER_SIZE)
		{
			printf("Error! Func %s Line %d\n", __func__, __LINE__);
			session->SetSessionDead();
			return;
		}

		if (header.code != dfPACKET_HEADER_CODE)
		{
			printf("Error! Wrong Header Code! - Func %s Line %d\n", __func__, __LINE__);
			session->SetSessionDead();
			return;
		}

		if (iUsedSize < dfPACKET_HEADER_SIZE + header.payload_size)
			break;

		int moveReadRet = session->_recvRingBuf.MoveReadPos(dfPACKET_HEADER_SIZE);
		if (moveReadRet != dfPACKET_HEADER_SIZE)
		{
			printf("Error! Func %s Line %d\n", __func__, __LINE__);
			session->SetSessionDead();
			return;
		}

		try
		{
			bool handlePacketRet = HandleCSPackets(pPlayer, header.action_type);
		}
		catch (...) {
			printf("CSPacket_ Buffer Error. Func %s, Line %d\n", __func__, __LINE__);
			return;
		}
		iUsedSize = session->_recvRingBuf.GetUseSize();
	}
}

bool NetworkManager::HandleCSPackets(Player* pPlayer, UINT8 type)
{
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
		return HandleCSPacket_MoveStart(pPlayer);
		break;

	case dfPACKET_CS_MOVE_STOP:
		return HandleCSPacket_MoveStop(pPlayer);
		break;

	case dfPACKET_CS_ATTACK1:
		return HandleCSPacket_Attack1(pPlayer);
		break;

	case dfPACKET_CS_ATTACK2:
		return HandleCSPacket_Attack2(pPlayer);
		break;

	case dfPACKET_CS_ATTACK3:
		return HandleCSPacket_Attack3(pPlayer);
		break;

	case dfPACKET_CS_ECHO:
		return HandleCSPacket_ECHO(pPlayer);
		break;
	}

	return false;
}

bool NetworkManager::HandleCSPacket_MoveStart(Player* pPlayer)
{
	char moveDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(moveDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		pPlayer->GetSession()->SetSessionDead();
		return false;
	}
	pPlayer->GetSession()->_recvSerialPacket.MoveWritePos(size);

	pPlayer->GetSession()->_recvSerialPacket >> moveDirection;
	pPlayer->GetSession()->_recvSerialPacket >> X;
	pPlayer->GetSession()->_recvSerialPacket >> Y;

	if (abs(X - pPlayer->GetX()) > dfERROR_RANGE ||
		abs(Y - pPlayer->GetY()) > dfERROR_RANGE)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int setRet = SetSCPacket_SYNC(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY());
		NetworkManager::GetInstance().EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
	}

	pPlayer->SetPlayerMoveStart(moveDirection, X, Y);

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
pPlayer->GetID(), pPlayer->GetHeadDirection(), X, Y, moveDirection, pPlayer->GetX(), pPlayer->GetY());
#endif

		pPlayer->GetSession()->_sendSerialPacket.Clear();
	int setRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket, 
		pPlayer->GetID(), pPlayer->GetMoveDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSector(), pPlayer->GetSession());
	return true;
}

bool NetworkManager::HandleCSPacket_MoveStop(Player* pPlayer)
{
	char headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		pPlayer->GetSession()->SetSessionDead();
		return false;
	}
	pPlayer->GetSession()->_recvSerialPacket.MoveWritePos(size);

	pPlayer->GetSession()->_recvSerialPacket >> headDirection;
	pPlayer->GetSession()->_recvSerialPacket >> X;
	pPlayer->GetSession()->_recvSerialPacket >> Y;

	if (abs(X - pPlayer->GetX()) > dfERROR_RANGE ||
		abs(Y - pPlayer->GetY()) > dfERROR_RANGE)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int setRet = SetSCPacket_SYNC(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY());
		NetworkManager::GetInstance().EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		//SetStateDead();
	}

	pPlayer->SetPlayerMoveStop(headDirection, X, Y);

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
pPlayer->GetID(), headDirection, X, Y, pPlayer->GetMoveDirection(), pPlayer->GetX(), pPlayer->GetY());
#endif
		pPlayer->GetSession()->_sendSerialPacket.Clear();
	int setRet = SetSCPacket_MOVE_STOP(&pPlayer->GetSession()->_sendSerialPacket, 
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSector(), pPlayer->GetSession());
	return true;
}

bool NetworkManager::HandleCSPacket_Attack1(Player* pPlayer)
{
	char headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		pPlayer->GetSession()->SetSessionDead();
		return false;
	}
	pPlayer->GetSession()->_recvSerialPacket.MoveWritePos(size);

	pPlayer->GetSession()->_recvSerialPacket >> headDirection;
	pPlayer->GetSession()->_recvSerialPacket >> X;
	pPlayer->GetSession()->_recvSerialPacket >> Y;


	if (abs(X - pPlayer->GetX()) > dfERROR_RANGE ||
		abs(Y - pPlayer->GetY()) > dfERROR_RANGE)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int setRet = SetSCPacket_SYNC(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY());
		NetworkManager::GetInstance().EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack1(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK1(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}
#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 1\n\n\
packetAttack1.headDirection: %d\n\
packetAttack1.X: %d\n\
packetAttack1.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
pPlayer->GetID(), headDirection, X, Y, pPlayer->GetX(), pPlayer->GetY());
#endif
	return true;
}

bool NetworkManager::HandleCSPacket_Attack2(Player* pPlayer)
{
	char headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		pPlayer->GetSession()->SetSessionDead();
		return false;
	}
	pPlayer->GetSession()->_recvSerialPacket.MoveWritePos(size);

	pPlayer->GetSession()->_recvSerialPacket >> headDirection;
	pPlayer->GetSession()->_recvSerialPacket >> X;
	pPlayer->GetSession()->_recvSerialPacket >> Y;

	if (abs(X - pPlayer->GetX()) > dfERROR_RANGE ||
		abs(Y - pPlayer->GetY()) > dfERROR_RANGE)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int setRet = SetSCPacket_SYNC(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY());
		NetworkManager::GetInstance().EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack2(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK2(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}

#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 2\n\n\
packetAttack2.headDirection: %d\n\
packetAttack2.X: %d\n\
packetAttack2.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
pPlayer->GetID(), headDirection, X, Y, pPlayer->GetX(), pPlayer->GetY());
#endif
	return true;
}

bool NetworkManager::HandleCSPacket_Attack3(Player* pPlayer)
{
	char headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
		pPlayer->GetSession()->SetSessionDead();
		return false;
	}
	pPlayer->GetSession()->_recvSerialPacket.MoveWritePos(size);

	pPlayer->GetSession()->_recvSerialPacket >> headDirection;
	pPlayer->GetSession()->_recvSerialPacket >> X;
	pPlayer->GetSession()->_recvSerialPacket >> Y;

	if (abs(X - pPlayer->GetX()) > dfERROR_RANGE ||
		abs(Y - pPlayer->GetY()) > dfERROR_RANGE)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int setRet = SetSCPacket_SYNC(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY());
		NetworkManager::GetInstance().EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		printf("Error! Func %s Line %d\n", __func__, __LINE__);
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack3(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK3(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}

#ifdef RECV_PACKET_DEBUG
	printf("===================================\n\
%d: ATTACK 3\n\n\
packetAttack3.headDirection: %d\n\
packetAttack3.X: %d\n\
packetAttack3.Y: %d\n\n\
now X: %d\n\
now Y: %d\n\
====================================\n\n",
pPlayer->GetID(), headDirection, X, Y, pPlayer->GetX(), pPlayer->GetY());
#endif
	return true;
}

bool NetworkManager::GetCSPacket_ECHO(SerializePacket* pPacket, RingBuffer* recvRBuffer, int& time)
{
	int size = sizeof(time);
	int dequeueRet = recvRBuffer->Dequeue(pPacket->GetWritePtr(), size);
	if (dequeueRet != size)
	{
		/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		dump.Crash();*/
		return false;
	}
	pPacket->MoveWritePos(dequeueRet);
	*pPacket >> time;
	return true;
}

bool NetworkManager::HandleCSPacket_ECHO(Player* pPlayer)
{
	int time;

	pPlayer->GetSession()->_recvSerialPacket.Clear();

	bool getRet = GetCSPacket_ECHO(&pPlayer->GetSession()->_recvSerialPacket, &pPlayer->GetSession()->_recvRingBuf, time);
	if (!getRet) return false;
	
	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int setRet = SetSCPacket_ECHO(&pPlayer->GetSession()->_sendSerialPacket, time);
	EnqMsgUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
	
	return true;
}


void NetworkManager::DisconnectDeadSessions()
{
	for (int i = 0; i < _disconnectCnt; i++)
	{
		int ID = _disconnectSessionIDs[i];

		Player* pPlayer =IngameManager::GetInstance()._Players[ID];
		if (pPlayer == nullptr)
		{
			/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d player is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			::wprintf(L"%s[%d] Session %d player is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			dump.Crash();*/
			return;
		}
		IngameManager::GetInstance()._Players[ID] = nullptr;

		// Remove from Sector
		vector<Player*>::iterator vectorIter = pPlayer->GetSector()->_players.begin();
		for (; vectorIter < pPlayer->GetSector()->_players.end(); vectorIter++)
		{
			if ((*vectorIter) == pPlayer)
			{
				pPlayer->GetSector()->_players.erase(vectorIter);
				break;
			}
		}
		
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int deleteRet = SetSCPacket_DELETE_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket, pPlayer->GetID());
		IngameManager::GetInstance().EnqMsgAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), deleteRet, pPlayer->GetSector());
		IngameManager::GetInstance()._pPlayerPool->Free(pPlayer);
		
		Session* pSession = _Sessions[ID];
		if (pSession == nullptr)
		{
			/*LOG(L"FightGame", CSystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			::wprintf(L"%s[%d] Session %d is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			dump.Crash();*/
			return;
		}
		_Sessions[ID] = nullptr;

		closesocket(pSession->_socket);	
		_pSessionPool->Free(pSession);
		_usableSessionID[_usableCnt++] = ID;
	}

//	_disconnectCnt = 0;
}



//serialize Buffer·Î ¹Ù²î¸é¼­ msg·Î header + msg°¡ µé¾î¿È.
void NetworkManager::EnqMsgUnicast(char* msg, int size, Session* session)
{
	int enqueueRet = session->_sendRingBuf.Enqueue(msg, size);
	if (enqueueRet != size)
	{
		printf("Error! Function %s Line %d\n", __func__, __LINE__);
		session->SetSessionDead();
	}
}
