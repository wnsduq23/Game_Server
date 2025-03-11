#pragma once

#include "RingBuffer.h"
#include "SerializePacket.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"
#include "NetworkManager.h"
#pragma comment(lib, "ws2_32")


class Session
{
public:
	bool GetSessionAlive() { return _bAlive; }
	void SetSessionAlive() { _bAlive = true; }
	void SetSessionDead();
	inline void Initialize(DWORD ID)
	{
		_ID = ID;
		_bAlive = true;
		_recvRingBuf.ClearBuffer();
		_sendRingBuf.ClearBuffer();
	}
public:
	DWORD		_ID;
	bool		_bAlive;
	SOCKET		_socket;
	SOCKADDR_IN	_addr;
	RingBuffer	_sendRingBuf;
	RingBuffer	_recvRingBuf;
	SerializePacket _recvSerialPacket;
	SerializePacket _sendSerialPacket;
	DWORD _lastRecvTime;
};
