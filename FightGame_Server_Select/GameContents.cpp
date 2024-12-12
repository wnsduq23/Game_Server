#include "GameContents.h"
#include "PlayerManager.h"
#include <Windows.h>


bool SkipForFixedFrame(void)
{
	timeBeginPeriod(1);
	static DWORD oldTick = timeGetTime();
	//너무 시간이 많이 안 지났으면 skip
	if ((timeGetTime() - oldTick) < (1000 / FPS))
		return true;
	//다음 프레임 시점 계산
	oldTick += (1000 / FPS);
	return false;

}

void GameContentsModule()
{
	// 프레임 설정 
	if (SkipForFixedFrame()) return;

	// 더 확장성을 고려한다면 여기에 몬스터, 이벤트 등등이 들어올 듯

	// 플레이어 이동 및 공격 처리
	PlayerManager::GetInstance()->PlayerActionProc();
}