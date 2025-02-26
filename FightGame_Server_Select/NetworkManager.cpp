#include "NetworkManager.h"
#include "IngameManager.h"
#include "main.h"
#include <stdio.h>
#include "SetSCPacket.h"

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
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: WSAStartup Error\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: WSAStartup Error\n",
			_T(__FUNCTION__), __LINE__);

		g_dump.Crash();
		return;
	}

	// Create Socket
	_listensock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listensock == INVALID_SOCKET)
	{
		err = WSAGetLastError();

		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: listen sock is INVALIED, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: listen sock is INVALIED, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
		return;
	}

	// Bind
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 모든 네트워크 인터페이스에서 수신
	serveraddr.sin_port = htons(dfNETWORK_PORT);

	bindRet = bind(_listensock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (bindRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();

		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: bind Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: bind Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
		return;
	}

	// Listen
	listenRet = listen(_listensock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();

		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: listen Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: listen Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
		return;
	}

	// Set Non-Blocking Mode
	u_long on = 1;
	ioctRet = ioctlsocket(_listensock, FIONBIO, &on);
	if (ioctRet == SOCKET_ERROR)
	{
		err = WSAGetLastError();

		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: ioct Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: ioct Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
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
		
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: select Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: select Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
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
		if (err == WSAECONNRESET || err == WSAECONNABORTED)
		{
			session->SetSessionDead();
			return;
		}
		else if (err != WSAEWOULDBLOCK)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: Session %d Send Error, %d\n",
				_T(__FUNCTION__), __LINE__, session->_ID, err);

			::wprintf(L"%s[%d]: Session %d Send Error, %d\n",
				_T(__FUNCTION__), __LINE__, session->_ID, err);

			session->SetSessionDead();
			return;
		}
	}

	int moveRet = session->_sendRingBuf.MoveReadPos(sendRet);
	if (sendRet != moveRet)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] Session %d - sendRBuf moveReadPos Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, sendRet, moveRet);

		::wprintf(L"%s[%d] Session %d - sendRBuf moveReadPos Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, sendRet, moveRet);

		g_dump.Crash();
		return;
	}
}

void NetworkManager::AcceptProc()
{
	// Session Num is more than SESSION_MAX
	if (_usableCnt == 0 && _sessionIDs == dfSESSION_MAX)
	{
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);

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
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: new Error, %d\n",
			_T(__FUNCTION__), __LINE__, ID);

		::wprintf(L"%s[%d]: new Error, %d\n",
			_T(__FUNCTION__), __LINE__, ID);

		g_dump.Crash();	
		return;
	}

	int addrlen = sizeof(pSession->_addr);
	pSession->_socket = accept(_listensock, (SOCKADDR*)&pSession->_addr, &addrlen);
	if (pSession->_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: accept Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		::wprintf(L"%s[%d]: accept Error, %d\n",
			_T(__FUNCTION__), __LINE__, err);

		g_dump.Crash();
		return;
	}
	linger lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;
	setsockopt(pSession->_socket, SOL_SOCKET, SO_LINGER, (const char*)&lingerOption, sizeof(lingerOption));

	pSession->_lastRecvTime = GetTickCount64();
	_Sessions[pSession->_ID] = pSession;
	IngameManager::GetInstance().CreatePlayer(pSession);

}

void NetworkManager::RecvProc(Session* session)
{
	session->_lastRecvTime = GetTickCount64();
	int recvRet = 0;
	
	int directEnqueue = session->_recvRingBuf.DirectEnqueueSize();
	if (directEnqueue != 0)
	{
		recvRet = recv(session->_socket,
			session->_recvRingBuf.GetWritePtr(),
			directEnqueue, 0);
	}
	else
	{
		int freeSize = session->_recvRingBuf.GetFreeSize();
		if (freeSize != 0)
		{
			recvRet = recv(session->_socket,
				session->_recvRingBuf.GetWritePtr(),
				freeSize, 0);
		}
		else
		{
			session->_recvRingBuf.Resize(session->_recvRingBuf.GetBufferSize() * 1.5f);
			// 업데이트된 DirectEnqueueSize()를 캐싱
			directEnqueue = session->_recvRingBuf.DirectEnqueueSize();
			recvRet = recv(session->_socket,
				session->_recvRingBuf.GetWritePtr(),
				directEnqueue, 0);
		}
	}

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAECONNRESET || err == WSAECONNABORTED)
		{
			session->SetSessionDead();
			return;
		}
		else if (err != WSAEWOULDBLOCK)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: Session %d recv Error, %d\n",
				_T(__FUNCTION__), __LINE__,session->_ID, err);

			::wprintf(L"%s[%d]: Session %d recv Error, %d\n",
				_T(__FUNCTION__), __LINE__, session->_ID, err);

			session->SetSessionDead();
			return;
		}
	}
	else if (recvRet == 0)
	{
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d]: recv returns 0\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: recv returns 0\n",
			_T(__FUNCTION__), __LINE__);

		session->SetSessionDead();

		return;
	}

	int moveRet = session->_recvRingBuf.MoveWritePos(recvRet);
	if (recvRet != moveRet)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] Session %d - recvRBuf moveWritePos Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, recvRet, moveRet);

		::wprintf(L"%s[%d] Session %d - recvRBuf moveWritePos Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, recvRet, moveRet);

		g_dump.Crash();
		return;
	}

	Player* pPlayer = IngameManager::GetInstance()._Players[session->_ID];
	if (pPlayer == nullptr)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] Session %d player is nullptr\n",
			_T(__FUNCTION__), __LINE__, session->_ID);

		::wprintf(L"%s[%d] Session %d player is nullptr\n",
			_T(__FUNCTION__), __LINE__, session->_ID);

		g_dump.Crash();
		return;
	}
	int iUsedSize = session->_recvRingBuf.GetUseSize();
	while (iUsedSize > 0)
	{
		if (iUsedSize <= dfPACKET_HEADER_SIZE)
			break;

		stPACKET_HEADER header;
		int peekRet = session->_recvRingBuf.Peek((char*)&header, dfPACKET_HEADER_SIZE);
		if (peekRet != dfPACKET_HEADER_SIZE)//setsessiondead 안하는 이유 : 
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]  Session %d - recvRBuf Peek Error (req - %d, ret - %d)\n",
				_T(__FUNCTION__), __LINE__, session->_ID, dfPACKET_HEADER_SIZE, peekRet);

			::wprintf(L"%s[%d]  Session %d - recvRBuf Peek Error (req - %d, ret - %d)\n",
				_T(__FUNCTION__), __LINE__, session->_ID, dfPACKET_HEADER_SIZE, peekRet);

			g_dump.Crash();
			return;
		}

		if (header.code != dfPACKET_HEADER_CODE)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: Session %d Wrong Header Code Error, %x\n",
				_T(__FUNCTION__), __LINE__, session->_ID, header.code);

			::wprintf(L"%s[%d]: Session %d Wrong Header Code Error, %x\n",
				_T(__FUNCTION__), __LINE__, session->_ID, header.code);

			session->SetSessionDead();
			return;
		}

		if (iUsedSize < dfPACKET_HEADER_SIZE + header.payload_size)
			break;

		int moveReadRet = session->_recvRingBuf.MoveReadPos(dfPACKET_HEADER_SIZE);
		if (moveReadRet != dfPACKET_HEADER_SIZE)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d - recvRBuf moveReadPos Error (req - %d, ret - %d)\n",
				_T(__FUNCTION__), __LINE__, session->_ID, dfPACKET_HEADER_SIZE, moveReadRet);

			::wprintf(L"%s[%d] Session %d - recvRBuf moveReadPos Error (req - %d, ret - %d)\n",
				_T(__FUNCTION__), __LINE__, session->_ID, dfPACKET_HEADER_SIZE, moveReadRet);

			g_dump.Crash();
			return;
		}

		bool handlePacketRet = HandleCSPackets(pPlayer, header.action_type);
		if (!handlePacketRet)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: Session %d Handle CS Packet Error\n",
				_T(__FUNCTION__), __LINE__, session->_ID);

			::wprintf(L"%s[%d]: Session %d Handle CS Packet Error\n",
				_T(__FUNCTION__), __LINE__, session->_ID);

			session->SetSessionDead();
			return;
		}

		iUsedSize = session->_recvRingBuf.GetUseSize();
	}
}

bool NetworkManager::HandleCSPackets(Player* pPlayer, BYTE type)
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

	LOG(L"FightGame", SystemLog::ERROR_LEVEL,
		L"%s[%d] No Switch Case, %d\n", _T(__FUNCTION__), __LINE__, type);
	::wprintf(L"%s[%d] No Switch Case, %d\n", _T(__FUNCTION__), __LINE__, type);
	return false;
}

bool NetworkManager::HandleCSPacket_MoveStart(Player* pPlayer)
{
	BYTE moveDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(moveDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
		NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));

		::wprintf(L"%s[%d] SessionID : %d (socket port :%hd)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));
	}

	pPlayer->SetPlayerMoveStart(moveDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int setRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket, 
		pPlayer->GetID(), pPlayer->GetMoveDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSector(), pPlayer->GetSession());
	return true;
}

bool NetworkManager::HandleCSPacket_MoveStop(Player* pPlayer)
{
	BYTE headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
		NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));

		::wprintf(L"%s[%d] SessionID : %d (socket port :%d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));
	}

	pPlayer->SetPlayerMoveStop(headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int setRet = SetSCPacket_MOVE_STOP(&pPlayer->GetSession()->_sendSerialPacket, 
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSector(), pPlayer->GetSession());
	return true;
}

bool NetworkManager::HandleCSPacket_Attack1(Player* pPlayer)
{
	BYTE headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
		NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));

		::wprintf(L"%s[%d] SessionID : %d (socket port :%d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack1(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK1(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}
	return true;
}

bool NetworkManager::HandleCSPacket_Attack2(Player* pPlayer)
{
	BYTE headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
		NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();

		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));

		::wprintf(L"%s[%d] SessionID : %d (socket port :%d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack2(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK2(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}

	return true;
}

bool NetworkManager::HandleCSPacket_Attack3(Player* pPlayer)
{
	BYTE headDirection;
	short X;
	short Y;

	pPlayer->GetSession()->_recvSerialPacket.Clear();
	int size = sizeof(headDirection) + sizeof(X) + sizeof(Y);
	int dequeueRet = pPlayer->GetSession()->_recvRingBuf.Dequeue(pPlayer->GetSession()->_recvSerialPacket.GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
		NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));

		::wprintf(L"%s[%d] SessionID : %d (socket port :%d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port));
	}

	Player* damagedPlayer = nullptr;
	pPlayer->SetPlayerAttack3(damagedPlayer, headDirection, X, Y);

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int attackSetRet = SetSCPacket_ATTACK3(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY());
	IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), attackSetRet, pPlayer->GetSector());

	if (damagedPlayer != nullptr)
	{
		pPlayer->GetSession()->_sendSerialPacket.Clear();
		int damageSetRet = SetSCPacket_DAMAGE(&pPlayer->GetSession()->_sendSerialPacket,
			pPlayer->GetID(), damagedPlayer->GetID(), damagedPlayer->GetHp());
		IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), damageSetRet, damagedPlayer->GetSector());
	}

	return true;
}

bool NetworkManager::GetCSPacket_ECHO(SerializePacket* pPacket, RingBuffer* recvRBuffer, int& time)
{
	int size = sizeof(time);
	int dequeueRet = recvRBuffer->Dequeue(pPacket->GetWritePtr(), size);
	if (dequeueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		::wprintf(L"%s[%d] recvRBuf Dequeue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, size, dequeueRet);

		g_dump.Crash();
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
	SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), setRet, pPlayer->GetSession());
	
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
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d player is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			::wprintf(L"%s[%d] Session %d player is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			g_dump.Crash();
			return;
		}
		/*if (pPlayer->GetHp() > 0 && GetTickCount64() - pPlayer->GetSession()->_lastRecvTime < dfNETWORK_PACKET_RECV_TIMEOUT)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d player is not HP under 0\n",
				_T(__FUNCTION__), __LINE__, ID);

			::wprintf(L"%s[%d] Session %d player is not HP under 0\n",
				_T(__FUNCTION__), __LINE__, ID);

			g_dump.Crash();
			return;
		}*/
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
		IngameManager::GetInstance().SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), deleteRet, pPlayer->GetSector());
		IngameManager::GetInstance()._pPlayerPool->Free(pPlayer);
		
		Session* pSession = _Sessions[ID];
		if (pSession == nullptr)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d] Session %d is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			::wprintf(L"%s[%d] Session %d is nullptr\n",
				_T(__FUNCTION__), __LINE__, ID);

			g_dump.Crash();
			return;
		}
		_Sessions[ID] = nullptr;
		closesocket(pSession->_socket);
		_pSessionPool->Free(pSession);
		_usableSessionID[_usableCnt++] = ID;
	}

	_disconnectCnt = 0;
}



//serialize Buffer로 바뀌면서 msg로 header + msg가 들어옴.
void NetworkManager::SendPacketUnicast(char* msg, int size, Session* session)
{
	if (session == nullptr)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] Session is nullptr\n", _T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d] Session is nullptr\n", _T(__FUNCTION__), __LINE__);

		g_dump.Crash();
		return;
	}
	int enqueueRet = session->_sendRingBuf.Enqueue(msg, size);
	if (enqueueRet != size)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, size, enqueueRet);

		::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
			_T(__FUNCTION__), __LINE__, session->_ID, size, enqueueRet);

		g_dump.Crash();
		return;
	}
}
