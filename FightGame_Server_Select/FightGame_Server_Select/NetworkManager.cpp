﻿#include "NetworkManager.h"
#include "IngameManager.h"
#include "main.h"
#include "Profiler.h"
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
	int flag = 1;
	setsockopt(_listensock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

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

	_time.tv_sec = 0;
	_time.tv_usec = 0;
	_addrlen = sizeof(SOCKADDR_IN);

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
    // 3) select로 처리된 결과 중, 종료될 세션이 있다면 여기서 정리
    PRO_BEGIN(L"Delayed Disconnect");
    DisconnectDeadSessions();
    PRO_END(L"Delayed Disconnect");

    PRO_BEGIN(L"Network");

    // 1) 읽기(r) / 쓰기(w) 세션 목록 구성
    int rCount = 0;
    int wCount = 0;

    for (int i = 0; i < dfSESSION_MAX; i++)
    {
        // 세션이 살아있는 상태인지 확인
        if (_Sessions[i] == nullptr || !_Sessions[i]->_bAlive)
            continue;

        // 읽기 대상
        _rSessions[rCount++] = _Sessions[i];

        // 쓰기 버퍼에 보낼 데이터가 있으면 쓰기 대상
        if (_Sessions[i]->_sendRingBuf.GetUseSize() > 0)
            _wSessions[wCount++] = _Sessions[i];
    }

	if (rCount == 0 && wCount == 0) // 처음 accept만 해야할 때
	{
		SelectModel(0, 0, 0, 0);
	}
    // 2) FD_SETSIZE에 맞춰 세션들을 잘라가며 SelectModel() 호출
    //    보통 Windows에서 FD_SETSIZE는 64이며,
    //    읽기는 (FD_SETSIZE - 1), 쓰기는 FD_SETSIZE 로 잡는 식
    const int READ_CHUNK  = FD_SETSIZE - 1; 
    const int WRITE_CHUNK = FD_SETSIZE;

    int rPos = 0;
    int wPos = 0;

    while ( (rPos < rCount) || (wPos < wCount) )
    {
        // 이번에 처리할 읽기 세션 개수
        int rSize = (rPos < rCount)
                   ? min(READ_CHUNK, (rCount - rPos))
                   : 0;

        // 이번에 처리할 쓰기 세션 개수
        int wSize = (wPos < wCount)
                   ? min(WRITE_CHUNK, (wCount - wPos))
                   : 0;

        // 처리할 세션이 하나도 없으면 break
        if (rSize == 0 && wSize == 0)
            break;

        // 부분적으로 select 호출
        // - _rSessions[rPos] ~ _rSessions[rPos + rSize - 1]
        // - _wSessions[wPos] ~ _wSessions[wPos + wSize - 1]
        SelectModel(rPos, rSize, wPos, wSize);

        // 처리한 세션만큼 인덱스 이동
        rPos += rSize;
        wPos += wSize;
    }

    PRO_END(L"Network");

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
	int selectRet = select(0, &_rset, &_wset, NULL, &_time);
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
	else if (selectRet > 0) // Handle Selected Socket
	{
		//printf("rCount : %d / wCount : %d\n", rCount, wCount);
		if (FD_ISSET(_listensock, &_rset))
			AcceptProc();

		for (int i = 0; i < rCount; i++)
			if (FD_ISSET(_rSessions[rStartIdx + i]->_socket, &_rset))
				RecvProc(_rSessions[rStartIdx + i]);

		for (int i = 0; i < wCount; i++)
			if (FD_ISSET(_wSessions[wStartIdx + i]->_socket, &_wset) && _wSessions[wStartIdx + i]->_bAlive)
				SendProc(_wSessions[wStartIdx + i]);

	}
}

void NetworkManager::SendProc(Session* session)
{
	PRO_BEGIN(L"Network: Send");

	if (session->_sendRingBuf.GetUseSize() <= 25)
		return;

	int sendRet = 0;
	int directDeqSize;
	int moveRet;
	directDeqSize = session->_sendRingBuf.DirectDequeueSize();
	if (directDeqSize != session->_sendRingBuf.GetUseSize())
	{
		sendRet = send(session->_socket,
			session->_sendRingBuf.GetReadPtr(),
			directDeqSize, 0);

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
		moveRet = session->_sendRingBuf.MoveReadPos(sendRet);
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
		sendRet = send(session->_socket,
			session->_sendRingBuf.GetReadPtr(),
			session->_sendRingBuf.GetUseSize(), 0);
	}
	else
	{
		sendRet = send(session->_socket,
			session->_sendRingBuf.GetReadPtr(),
			directDeqSize, 0);
	}
	PRO_END(L"Network: Send");

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

	moveRet = session->_sendRingBuf.MoveReadPos(sendRet);
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
	static DWORD oldTick = timeGetTime();
	PRO_BEGIN(L"Network : Accept");
	if (_usableCnt == 0 && _sessionIDs == dfSESSION_MAX)
	{
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: usableCnt = 0, sessionIDs = MAX\n",
			_T(__FUNCTION__), __LINE__);
		return;
	}

	DWORD ID;
	if (_usableCnt == 0)
		ID = _sessionIDs++;
	else
		ID = _usableSessionID[--_usableCnt];

	Session* pSession = _pSessionPool->Alloc();
	pSession->Initialize(ID);
	if (pSession == nullptr)
	{
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: new Error\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"%s[%d]: new Error\n",
			_T(__FUNCTION__), __LINE__);
		return;
	}

	pSession->_socket = accept(_listensock, (SOCKADDR*)&pSession->_addr, &_addrlen);
	/*b++;
	if (timeGetTime() - oldTick > 1000)
	{
		::wprintf(L"%s[%d]: accept : %d\n", _T(__FUNCTION__), __LINE__, b);
		b = 0;
		oldTick += 1000;
	}*/
	if (pSession->_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			_usableSessionID[_usableCnt++] = ID;
			_pSessionPool->Free(pSession);
			return;
		}
		else
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: accept Error, %d\n", _T(__FUNCTION__), __LINE__, err);
			::wprintf(L"%s[%d]: accept Error, %d\n", _T(__FUNCTION__), __LINE__, err);
			_pSessionPool->Free(pSession);
			return;
		}
	}

	LINGER optval;
	optval.l_onoff = 1;
	optval.l_linger = 0;
	int optRet = setsockopt(pSession->_socket, SOL_SOCKET, SO_LINGER,
		(char*)&optval, sizeof(optval));
	pSession->_lastRecvTime = timeGetTime();
	_Sessions[pSession->_ID] = pSession;
	PRO_BEGIN(L"Ingame : CreatePlayer");
	IngameManager::GetInstance().CreatePlayer(pSession);
	PRO_END(L"Ingame : CreatePlayer");
	PRO_END(L"Network : Accept");
}

/*
* 링 버퍼에서 direct로 받을 수 있는 만큼만 받거나, writePtr이 맨 뒤면 
* ReadPtr까지 (= freesize)
*/
void NetworkManager::RecvProc(Session* session)
{
	session->_lastRecvTime = timeGetTime();
	PRO_BEGIN(L"Network: Recv");
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
			LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
				L"%s[%d]: Session %d Request recv Resize, %d\n",
				_T(__FUNCTION__), __LINE__,session->_ID);

			::wprintf(L"%s[%d]: Session %d Request recv Resize, %d\n",
				_T(__FUNCTION__), __LINE__, session->_ID);
			session->_recvRingBuf.Resize(session->_recvRingBuf.GetBufferSize() * 1.5f);
			// 업데이트된 DirectEnqueueSize()를 캐싱
			directEnqueue = session->_recvRingBuf.DirectEnqueueSize();
			recvRet = recv(session->_socket,
				session->_recvRingBuf.GetWritePtr(),
				directEnqueue, 0);
		}
	}

	PRO_END(L"Network: Recv");
	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAECONNRESET || err == WSAECONNABORTED) // RST 패킷
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
	else if (recvRet == 0)// FIN 패킷 
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
	// recvRingbuf에서 해당 세션으로 들어온 여러 패킷들 다 처리할 때 까지
	PRO_BEGIN(L"Network : HandlePacket");
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

		if (header.code != (BYTE)dfPACKET_HEADER_CODE)
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
	PRO_END(L"Network : HandlePacket");
}


void NetworkManager::DisconnectDeadSessions()
{
	for (int i = 0; i < _disconnectCnt; i++)
	{
		DWORD ID = _disconnectSessionIDs[i];

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



bool NetworkManager::HandleCSPackets(Player* pPlayer, BYTE type)
{
	//printf("Player : %d (%d,%d), Session : %d, action_type : %d\n", pPlayer->GetID(), pPlayer->GetX(), pPlayer->GetY(),pPlayer->GetSession()->_ID, type);
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
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%u) (%d, %d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port),X,Y);
		::wprintf(L"%s[%d] SessionID : %d (socket port :%u) (%d, %d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);

		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		PRO_SAVE(L"output.txt");
		g_dump.Crash();
	}

	pPlayer->SetPlayerMoveStart(moveDirection, X, Y);

	IngameManager::GetInstance().UpdateSector(pPlayer);

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
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%u) (%d, %d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port),X,Y);
		::wprintf(L"%s[%d] SessionID : %d (socket port :%u) (%d, %d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);

		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		PRO_SAVE(L"output.txt");
		g_dump.Crash();
	}

	pPlayer->SetPlayerMoveStop(headDirection, X, Y);

	IngameManager::GetInstance().UpdateSector(pPlayer); // 이걸 넣어야하나?

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
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%u) (%d, %d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);
		::wprintf(L"%s[%d] SessionID : %d (socket port :%u) (%d, %d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);

		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		PRO_SAVE(L"output.txt");
		g_dump.Crash();
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
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%u) (%d, %d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);
		::wprintf(L"%s[%d] SessionID : %d (socket port :%u) (%d, %d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);

		X = pPlayer->GetX();
		Y = pPlayer->GetY();

		PRO_SAVE(L"output.txt");
		g_dump.Crash();
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
		LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
			L"%s[%d] SessionID : %d (socket port :%u) (%d, %d) \n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);
		::wprintf(L"%s[%d] SessionID : %d (socket port :%u) (%d, %d)\n",
			_T(__FUNCTION__), __LINE__, pPlayer->GetSession()->_ID, ntohs(pPlayer->GetSession()->_addr.sin_port), X, Y);

		X = pPlayer->GetX();
		Y = pPlayer->GetY();
		PRO_SAVE(L"output.txt");
		g_dump.Crash();
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
