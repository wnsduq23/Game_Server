#include "Session.h"


void Session::SetSessionDead()
{
	if (_bAlive)
	{
		_bAlive = false;

		NetworkManager& nm = NetworkManager::GetInstance();
		nm._disconnectSessionIDs[nm._disconnectCnt] = _ID;
		nm._disconnectCnt++;
	}
}
