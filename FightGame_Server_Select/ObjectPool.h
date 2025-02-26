#pragma once
#include <new.h>
#include <stdlib.h>
#include <tchar.h>

//#define __OBJECT_POOL_DEBUG__

/* ===============================================

<< __OBJECT_POOL_DEBUG__ 설명 >>

1. Node

head와 tail에 검증 및 디버깅을 위한 값이 들어간다.
head, tail은 할당된 데이터 관리를 위한 것이므로
Alloc에서 값이 정해지고 Free에서 확인한다.

이 모드가 아닐 때는 tail에 다음 node의 주소값만 들어간다.

1) head

32bit일 경우, 상위 1byte로 Object pool ID를
하위 3byte로 Data 주소의 하위 3byte를 가진다.
64bit일 경우, 상위 4byte로 0x0000을,
하위 4byte로 32bit에서의 head 값을 가진다.

Object pool ID는 unsigned char(0~255)이며
그 이상의 Object pool이 만들어지는 것은 대비하지 않았다.

2) tail

tail는 해제되어 pool 안에 있을 때는 다음 node의 주소를,
할당되어 pool 밖에 있을 때는 head와 동일한 값을 가진다.

3) 검증 방법

Free가 호출되면 아래 사항들을 체크한다.
- head와 tail이 서로 같은 값을 가지는지
- pool ID와 instance ID를 바탕으로 나올 수 있는 값인지

만족하지 않을 경우, Pool에 속하지 않는 데이터의 해제 요청,
Overflow/Underflow 중 한가지 경우로 판단하여 예외를 던진다.

2. UseCount, Capacity

UseCount와 Capacity를 계산하여 필요할 때 출력할 수 있도록 한다.
또, Pool 소멸 시 UseCount를 확인하여 미반환 데이터가 있으면 알린다.
현재는 메시지를 콘솔에 출력하고 있으며 추후 변경할 예정이다.

==================================================*/

#ifdef __OBJECT_POOL_DEBUG__
#include <stdio.h>
extern unsigned char gObjectPoolID;
#endif


template <class DATA>
class ObjectPool
{
private:
#ifdef __OBJECT_POOL_DEBUG__

	struct stNODE
	{
		stNODE* head;
		DATA data;
		stNODE* tail;
	};

#else
	struct stNODE
	{
		DATA data;
		stNODE* tail = nullptr;
	};
#endif

public:
	template<typename... Types>
	ObjectPool(int iBlockNum, bool bPlacementNew, Types... args);
	virtual	~ObjectPool();

public:
	template<typename... Types>
	inline DATA* Alloc(Types... args);
	inline bool Free(DATA* pData);

private:
	bool _bPlacementNew;
	int _iBlockNum;
	stNODE* _pFreeNode = nullptr; // 할당되지 않고 사용 가능한 노드

#ifdef __OBJECT_POOL_DEBUG__
public:
	int		GetCapacityCount(void) { return _iCapacity; }
	int		GetUseCount(void) { return _iUseCount; }

private:
	int _iCapacity;
	int _iUseCount;
	unsigned char _iPoolID;
#endif

};

template<class DATA>
template<typename... Types>
ObjectPool<DATA>::ObjectPool(int iBlockNum, bool bPlacementNew, Types... args)
	:_bPlacementNew(bPlacementNew), _iBlockNum(iBlockNum), _pFreeNode(nullptr)
{
#ifdef __OBJECT_POOL_DEBUG__

	_iCapacity = _iBlockNum;
	_iUseCount = 0;
	_iPoolID = gObjectPoolID;
	gObjectPoolID++;
#endif

	if (_iBlockNum <= 0) return;

	if (_bPlacementNew)
	{
		// Alloc 시 Data의 생성자를 호출하므로 이때 호출하면 안된다

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (stNODE*)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (stNODE*)_pFreeNode;
			_pFreeNode = p;
		}
	}
	else
	{
		// Alloc 시 Data의 생성자를 호출하지 않으므로 이때 호출해야 된다

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (stNODE*)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			new (&(_pFreeNode->data)) DATA(args...);	//
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (stNODE*)_pFreeNode;
			_pFreeNode = p;
		}
		new (&(_pFreeNode->data)) DATA(args...);		//
	}
}

template<class DATA>
ObjectPool<DATA>::~ObjectPool()
{
#ifdef __OBJECT_POOL_DEBUG__

	if (_iUseCount != 0)
	{
		printf("There is Unfree Data!!\n");
		LOG(L"ERROR", SystemLog::ERROR_LEVEL, L"%s[%d]: There is Unfree Data\n", _T(__FUNCTION__), __LINE__);
	}

#endif

	if (_pFreeNode == nullptr)
		return;

	if (_bPlacementNew)
	{
		// Free 시 Data의 소멸자를 호출하므로 이때는 호출하면 안된다

		while (_pFreeNode->tail != (stNODE*)nullptr)
		{
			stNODE* next = _pFreeNode->tail;
			free(_pFreeNode);
			_pFreeNode = (stNODE*)next;
		}
		free(_pFreeNode);
	}
	else
	{
		// Free 시 Data의 소멸자를 호출하지 않으므로 이때 호출해야 된다

		while (_pFreeNode->tail != (stNODE*)nullptr)
		{
			stNODE* next = _pFreeNode->tail;
			(_pFreeNode->data).~DATA(); //
			free(_pFreeNode);
			_pFreeNode = (stNODE*)next;
		}
		(_pFreeNode->data).~DATA();		//
		free(_pFreeNode);
	}
}

template<class DATA>
template<typename... Types>
DATA* ObjectPool<DATA>::Alloc(Types... args)
{
	if (_pFreeNode == nullptr)
	{
		// 최초 생성인 경우 Data의 생성자를 호출한다

		stNODE* pNew = (stNODE*)malloc(sizeof(stNODE));
		new (&(pNew->data)) DATA(args...);

#ifdef __OBJECT_POOL_DEBUG__

		_iCapacity++;
		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(pNew->data));

		pNew->head = code;
		pNew->tail = code;

#endif

		return &(pNew->data);
	}

	if (_bPlacementNew)
	{
		// _pFreeNode에서 가져온 후 Data의 생성자를 호출한다

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;
		new (&(p->data)) DATA(args...);				//

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(p->data));

		p->head = code;
		p->tail = code;
#endif

		return &(p->data);
	}
	else
	{
		// _pFreeNode에서 가져온 후 Data의 생성자를 호출하지 않는다

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(p->data));

		p->head = code;
		p->tail = code;

#endif

		return &(p->data);
	}

	return nullptr;
}

template<class DATA>
bool ObjectPool<DATA>::Free(DATA* pData)
{
	if (_bPlacementNew)
	{
		// Data의 소멸자를 호출한 후 _pFreeNode에 push한다

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)pData;


		stNODE* offset = (stNODE*)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((stNODE*)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			printf("Error! code %o, head %o, tail %o\n",
				code, pNode->head, pNode->tail);
			LOG(L"ERROR", SystemLog::ERROR_LEVEL,
				L"%s[%d]: code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);
		}

		pData->~DATA();
		pNode->tail = (stNODE*)_pFreeNode;
		_pFreeNode = pNode;

#else

		pData->~DATA();								//
		((stNODE*)pData)->tail = (stNODE*)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	else
	{
		// Data의 소멸자를 호출하지 않고 _pFreeNode에 push한다

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)pData;

		stNODE* offset = (stNODE*)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((stNODE*)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			printf("Error! code %o, head %o, tail %o\n",
				code, pNode->head, pNode->tail);
			LOG(L"ERROR", SystemLog::ERROR_LEVEL,
				L"%s[%d]: code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);
			return false;
		}

		pNode->tail = (stNODE*)_pFreeNode;
		_pFreeNode = pNode;

#else

		((stNODE*)pData)->tail = (stNODE*)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	return false;
}
