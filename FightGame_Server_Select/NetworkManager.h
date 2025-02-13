#pragma once

#include "Session.h"
#include "Player.h"
#include "Protocol.h"
#include "ObjectPool.h"


/*========================
*		FUNCTION
========================*/


/*========================
*		CLASS
========================*/

class Player;

class NetworkManager
{
private:
	NetworkManager();
	~NetworkManager();
	std::list<Session*>		_sessionList;
	SOCKET					_listensock = INVALID_SOCKET;
	FD_SET					_rset;
	FD_SET					_wset;
public:
	// 복사 및 할당 방지
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;
private:
	ObjectPool<Session>* _pSessionPool;
	Session* _Sessions[dfSESSION_MAX];  // Use Session ID for Index
	Session* _rSessions[dfSESSION_MAX];
	Session* _wSessions[dfSESSION_MAX];
public:
	/*========================
	*	CLASS FUNCTION
	========================*/
	//싱글톤 인스턴스 접근
	static NetworkManager& GetInstance()
	{
		static NetworkManager _NetworkMgr;
		return _NetworkMgr;
	}
	bool HandleCSPackets(Player* pPlayer, UINT8 action_type);
	bool HandleCSPacket_MoveStart(Player* pPlayer);
	bool HandleCSPacket_MoveStop(Player* pPlayer);
	bool HandleCSPacket_Attack1(Player* pPlayer);
	bool HandleCSPacket_Attack2(Player* pPlayer);
	bool HandleCSPacket_Attack3(Player* pPlayer);
	bool HandleCSPacket_ECHO(Player* pPlayer);
	void EnqMsgUnicast(char* msg, int msgSize, Session* pSession);

	bool GetCSPacket_ECHO(SerializePacket* pPacket, RingBuffer* recvRBuffer, int& time);
	void SelectModel(int rStarIdx, int rCount, int wStartIdx, int wCount);
	void NetworkUpdate();
private:
	void AcceptProc();
	void RecvProc(Session* session);
	void SendProc(Session* session);

// session 관련
	int _sessionIDs = 0;
	int _usableCnt = 0;
	int _usableSessionID[dfSESSION_MAX];
	void DisconnectDeadSessions();
public:
	int _disconnectCnt = 0;
	int _disconnectSessionIDs[dfSESSION_MAX];
};

