#pragma once

constexpr int CACHE_SIZE = 512;
constexpr int CACHE_WAY = 8;
constexpr int CACHE_WAY_BIT = 3;

// L1 캐시 시뮬레이터
// L1 캐시 크기 32KB
// Index 9 - WAY_BIT, offset 6 : tag 나머지

__forceinline DWORD GetCacheIndex(UINT_PTR addr)
{
	return ((addr >> 6) & 0x3F);
}

__forceinline UINT_PTR GetCacheTag(UINT_PTR addr)
{
	UINT_PTR TAG_MASK = 0xFFF;
	return (addr & ~TAG_MASK);
}

struct CacheLine
{
public:
	CacheLine() = default;

	CacheLine(void* ptr)
	{
#ifdef _WIN64
		offset = (unsigned __int64)ptr & 0x3F;
		index = ((unsigned __int64)ptr >> 6) & 0x3F;
		tag = (unsigned __int64)ptr >> 12;
#else
		offset = (unsigned int)ptr & 0x3F;
		index = ((unsigned int)ptr >> 6) & 0x3F;
		tag = (unsigned int)ptr >> 12;
#endif
	}


#ifdef _WIN64
	unsigned __int64 offset;
	unsigned __int64 index;
	unsigned __int64 tag;
#else
	unsigned int offset;
	unsigned int index;
	unsigned int tag;
#endif
};

class CacheSimulator
{
public:
	CacheSimulator()
	{

	}

	bool IsCacheHit(void* ptr)
	{
		CacheLine cl(ptr);

		// 먼저 캐시에 있는지 찾는다.
		int accessIndex = -1;
		for (size_t i = 0; i < 8; i++)
		{
			if (m_arrCacheL1[cl.index * CACHE_WAY + i].tag == cl.tag)
			{
				accessIndex = i;
			}
		}

		// 캐시 히트
		if (accessIndex != -1)
		{
			CacheAccess(accessIndex, cl.index);
			return true;
		}
		// 캐시 미스
		// 교체 정책 수행

		accessIndex = CacheExchange(cl.index);
		m_arrCacheL1[accessIndex - CACHE_WAY + cl.index * CACHE_WAY] = cl;

		return false;
	}

	__forceinline void CacheAccess(int accessIndex, int index)
	{
		unsigned char route = m_arrCacheRoute[index];
		unsigned char indexBit = 0b10000000; // 7번째 비트부터 시작

		// accessIndex를 트리의 리프 노드 위치(8~15)로 변환
		accessIndex += CACHE_WAY;

		// 3단계 (CACHE_WAY_BIT = 3) 탐색
		for (int i = 0; i < CACHE_WAY_BIT; i++)
		{
			bool isRight = accessIndex % 2;  // 짝수면 왼쪽(0), 홀수면 오른쪽(1)
			accessIndex >>= 1; // 부모 노드로 이동

			unsigned char curIndex = (1 << (7 - accessIndex));  // 현재 비트 위치를 올바르게 설정

			if (isRight)
				route |= curIndex;  // 오른쪽으로 갔음을 기록
			else
				route &= ~curIndex; // 왼쪽으로 갔음을 기록
		}

		m_arrCacheRoute[index] = route;
	}

	__forceinline int CacheExchange(int index)
	{
		unsigned char route = m_arrCacheRoute[index];
		unsigned char indexBit = 0b10000000; // 7번째 비트

		int accessIndex = 1; // 트리의 루트부터 시작
		for (int i = 0; i < CACHE_WAY_BIT; i++)
		{
			unsigned char curIndex = (1 << (7 - accessIndex));  // 현재 비트 위치를 올바르게 설정

			// 현재 노드의 방향을 보고 교체할 노드를 결정
			if ((route & curIndex) == curIndex)  // 현재 노드가 오른쪽(1) 방향
			{
				route &= ~curIndex; // 왼쪽으로 변경 (0으로 설정)
				accessIndex = accessIndex << 1;  // 왼쪽 자식으로 이동
			}
			else  // 현재 노드가 왼쪽(0) 방향
			{
				route |= curIndex; // 오른쪽으로 변경 (1로 설정)
				accessIndex = (accessIndex << 1) + 1;  // 오른쪽 자식으로 이동
			}
		}

		m_arrCacheRoute[index] = route;
		return accessIndex;
	}


private:
	CacheLine m_arrCacheL1[CACHE_SIZE];

	// 경로 기록용
	unsigned char m_arrCacheRoute[CACHE_SIZE / CACHE_WAY] = { 0, };
};
