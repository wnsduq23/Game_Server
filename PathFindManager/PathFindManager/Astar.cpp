#include "AStar.h"
#include "Node.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
using namespace std;

void AStar::FindPath()
{
	if (!_bFindPathOn) return;

	if (_pNodeMgr->_pCurNode->_pos != _pNodeMgr->_pMap->_destPos)
	{
		if (_debugCreateNode)
			printf("\nCur Node (%d, %d)\n", _pNodeMgr->_pCurNode->_pos._x, _pNodeMgr->_pCurNode->_pos._y);
		
		_pNodeMgr->_closeList.push_back(_pNodeMgr->_pCurNode);
		_pNodeMgr->_pMap->SetMapState(
			_pNodeMgr->_pCurNode->_pos._x, _pNodeMgr->_pCurNode->_pos._y, Map::CLOSE);

		CreateNode(_pNodeMgr->_pCurNode); // 주변 노드(자식 노드) 생성

		if (_pNodeMgr->_openList.empty())// 앞으로 가 볼 곳이 없으면
		{
			printf("\nCan't Find Path!\n\n");
			_bFindPathOn = false;
			return;
		}

		// F( G + H)값을 기준으로 OpenList 정렬
		make_heap(_pNodeMgr->_openList.begin(), _pNodeMgr->_openList.end(), compareF);
		// 다음 단계 탐색은 OpenList 중 F값이 가장 낮은 노드
		_pNodeMgr->_pCurNode = _pNodeMgr->_openList.front();
		if (_debugOpenList) PrintOpenListForDebug();
		//_pCurNode로 넣었으니 openList에서 삭제시킴
		pop_heap(_pNodeMgr->_openList.begin(), _pNodeMgr->_openList.end());
		_pNodeMgr->_openList.pop_back();
	}
	else
	{
		_pNodeMgr->_pDest = _pNodeMgr->_pCurNode;
		if (_correctPath) CorrectPath();
		printf("\nComplete Find Path! (AStar: %d)\n\n", _pNodeMgr->_pDest->_g);
		_bFindPathOn = false;
	}
}

void AStar::FindPathStepInto()
{
	if (!_bFindPathStepOn)
	{
		_pNodeMgr->SetData();
		_bFindPathStepOn = true;
		_bFindPathOn = false;
	}

	if (_pNodeMgr->_pCurNode->_pos != _pNodeMgr->_pMap->_destPos)
	{
		if (_debugCreateNode)
			printf("\nCur Node (%d, %d)\n", _pNodeMgr->_pCurNode->_pos._x, _pNodeMgr->_pCurNode->_pos._y);
		
		_pNodeMgr->_closeList.push_back(_pNodeMgr->_pCurNode);
		_pNodeMgr->_pMap->SetMapState(
			_pNodeMgr->_pCurNode->_pos._x, _pNodeMgr->_pCurNode->_pos._y, Map::CLOSE);

		CreateNode(_pNodeMgr->_pCurNode);

		if (_pNodeMgr->_openList.empty())
		{
			printf("\nCan't Find Path!\n");
			_bFindPathStepOn = false;
			return;
		}

		make_heap(_pNodeMgr->_openList.begin(), _pNodeMgr->_openList.end(), compareF);
		_pNodeMgr->_pCurNode = _pNodeMgr->_openList.front();
		if (_debugOpenList) PrintOpenListForDebug();
		pop_heap(_pNodeMgr->_openList.begin(), _pNodeMgr->_openList.end());
		_pNodeMgr->_openList.pop_back();
	}
	else
	{
		_pNodeMgr->_pDest = _pNodeMgr->_pCurNode;
		if (_correctPath) CorrectPath();
		printf("\nComplete Find Path! (AStar: %d)\n\n", _pNodeMgr->_pDest->_g);
		_bFindPathStepOn = false;
	}
}

// pCurNode를 중심으로 주변 노드(자식 노드) 생성 및 관리
void AStar::CreateNode(Node* pCurNode)
{
	// 수직 수평 노드 생성
	for (int i = (int)DIR::UP; i <= (int)DIR::L; i++)
	{
		Pos newPos = pCurNode->_pos + _direction[i];
		Map::STATE state = _pNodeMgr->_pMap->GetMapState(newPos._x, newPos._y);

		switch (state)
		{
		case Map::NONE: // 탐색되지 않은 공간
		case Map::START:
		case Map::DEST:
		{
			Node* pNew = new Node(
				newPos,
				pCurNode->_g + VERT_DIST,
				newPos.GetDistanceToDest(_pNodeMgr->_pMap->_destPos),
				pCurNode
			);

			if (_debugCreateNode)
			{
				printf("%d. Create New Node (%d, %d) (parent: %d, %d) (%d + %d = %d)\n",
					i, pNew->_pos._x, pNew->_pos._y,
					pNew->_pParent->_pos._x, pNew->_pParent->_pos._y,
					pNew->_g, pNew->_h, pNew->_f);
			}
			_pNodeMgr->_openList.push_back(pNew);
			_pNodeMgr->_pMap->SetMapState(pNew->_pos._x, pNew->_pos._y, Map::OPEN);
		}
			break;

		case Map::OPEN:
		{
			compareG.pos = newPos;
			compareG.g = pCurNode->_g + VERT_DIST;

			vector<Node*>::iterator it = find_if(_pNodeMgr->_openList.begin(), 
				_pNodeMgr->_openList.end(), compareG);

			// G(= 시작점에서 해당 노드까지 도달하는 데 실제로 걸린 비용)
			// 값이 더 작은 노드를 찾았다면 다시 탐색 가능하게
			if (it != _pNodeMgr->_openList.end())
			{
				(*it)->ResetParent(pCurNode->_g + VERT_DIST, pCurNode);
				if (_debugCreateNode) printf("%d. Already Exist, Reset Parent (%d, %d) (parent: %d, %d)\n",
					i, newPos._x, newPos._y, (*it)->_pParent->_pos._x, (*it)->_pParent->_pos._y);
			}
			else
			{
				if (_debugCreateNode) printf("%d. Already Exist (%d, %d)\n", i, newPos._x, newPos._y);
			}	
		}
		break;

		case Map::CLOSE:
		{
			compareG.pos = newPos;
			compareG.g = pCurNode->_g + VERT_DIST;

			vector<Node*>::iterator it = find_if(_pNodeMgr->_closeList.begin(),
				_pNodeMgr->_closeList.end(), compareG);

			// G(= 시작점에서 해당 노드까지 도달하는 데 실제로 걸린 비용)
			// 값이 더 작은 노드를 찾았다면 다시 탐색 가능하게
			if (it != _pNodeMgr->_closeList.end())
			{
				(*it)->ResetParent(pCurNode->_g + VERT_DIST, pCurNode);
				if (_debugCreateNode) printf("%d. Already Exist, Reset Parent (%d, %d) (parent: %d, %d)\n",
					i, newPos._x, newPos._y, (*it)->_pParent->_pos._x, (*it)->_pParent->_pos._y);
			}
			else
			{
				if (_debugCreateNode) printf("%d. Already Exist (%d, %d)\n", i, newPos._x, newPos._y);
			}
		}
			break;

		case Map::OBSTACLE:
		case Map::RANGE_OUT:
			if (_debugCreateNode) printf("%d. Can't Go (%d, %d)\n", i, newPos._x, newPos._y);
			break;
		}
	}

	// 대각선 노드 생성
	for (int i = (int)DIR::UP_R; i <= (int)DIR::UP_L; i++)
	{
		Pos newPos = pCurNode->_pos + _direction[i];
		Map::STATE state = _pNodeMgr->_pMap->GetMapState(newPos._x, newPos._y);
		switch (state)
		{
		case Map::NONE:
		case Map::START:
		case Map::DEST:
		{
			Node* pNew = new Node(
				newPos,
				pCurNode->_g + DIAG_DIST,
				newPos.GetDistanceToDest(_pNodeMgr->_pMap->_destPos),
				pCurNode
			);

			if (_debugCreateNode)
			{
				printf("%d. Create New Node (%d, %d) (parent: %d, %d) (%d + %d = %d)\n",
					i, pNew->_pos._x, pNew->_pos._y,
					pNew->_pParent->_pos._x, pNew->_pParent->_pos._y,
					pNew->_g, pNew->_h, pNew->_f);
			}

			_pNodeMgr->_openList.push_back(pNew);
			_pNodeMgr->_pMap->SetMapState(pNew->_pos._x, pNew->_pos._y, Map::OPEN);
		}
			break;

		case Map::OPEN:
		{
			compareG.pos = newPos;
			compareG.g = pCurNode->_g + DIAG_DIST;

			vector<Node*>::iterator it = find_if(_pNodeMgr->_openList.begin(),
				_pNodeMgr->_openList.end(), compareG);

			if (it != _pNodeMgr->_openList.end())
			{
				(*it)->ResetParent(pCurNode->_g + DIAG_DIST, pCurNode);
				if (_debugCreateNode)  printf("%d. Already Exist, Reset Parent (%d, %d) (parent: %d, %d)\n",
					i, newPos._x, newPos._y, (*it)->_pParent->_pos._x, (*it)->_pParent->_pos._y);
			}
			else
			{
				if (_debugCreateNode)  printf("%d. Already Exist (%d, %d)\n", i, newPos._x, newPos._y);
			}

		}
		break;

		case Map::CLOSE:
		{
			compareG.pos = newPos;
			compareG.g = pCurNode->_g + DIAG_DIST;

			vector<Node*>::iterator it = find_if(_pNodeMgr->_closeList.begin(),
				_pNodeMgr->_closeList.end(), compareG);

			if (it != _pNodeMgr->_closeList.end())
			{
				(*it)->ResetParent(pCurNode->_g + DIAG_DIST, pCurNode);
				if (_debugCreateNode)  printf("%d. Already Exist, Reset Parent (%d, %d) (parent: %d, %d)\n",
					i, newPos._x, newPos._y, (*it)->_pParent->_pos._x, (*it)->_pParent->_pos._y);
			}
			else
			{
				if (_debugCreateNode)  printf("%d. Already Exist (%d, %d)\n", i, newPos._x, newPos._y);
			}
		}
		break;

		case Map::OBSTACLE:
		case Map::RANGE_OUT:
			if (_debugCreateNode)  printf("%d. Can't Go (%d, %d)\n", i, newPos._x, newPos._y);
			break;
		}
	}
}

bool AStar::CompareG::operator()(Node*& pNode) const
{
	return (pNode->_pos == pos && pNode->_g >= g);
}