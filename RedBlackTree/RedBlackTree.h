#pragma once

#include "Define.h"
#include "main.h"

template<typename DATA>
class RedBlackTree
{
	enum COLOR
	{
		BLACK = 0,
		RED
	};

public:
	class Node
	{
		friend RedBlackTree;

	private:
		Node(DATA data, Node* pChild)
			: _data(data), _color(RED), _pParent(nullptr),
			_pLeft(pChild), _pRight(pChild) {}

		~Node() {}

	public:
		DATA _data;

		COLOR _color;
		Node* _pParent;
		Node* _pLeft;
		Node* _pRight;
	};

public:
	Node* _pNil;
	Node* _pRoot;
	int _size = 0;

private:
	int _PrevBlack = 0;

public:
	RedBlackTree()
	{
		_pNil = new Node(0, nullptr);
		_pNil->_color = BLACK;
		_pRoot = _pNil;
	}
	~RedBlackTree() {}

public:
	int InsertNode(DATA data)
	{
		if (_size >= TREE_MAX)
		{
			return INSERT_ERROR_TREE_IS_FULL;
		}

		if (_pRoot == _pNil)
		{
			_pRoot = new Node(data, _pNil);
			_pRoot->_color = BLACK;
			_size++;
			//printf("==================\n");
			//printf("INSERT -----> %d  \n", data);
			//printf("==================\n");
			return INSERT_SUCCESS;
		}

		Node* pNode = _pRoot;

		for (;;)
		{
			if (pNode->_data == data)
			{
				return INSERT_ERROR_DUPLICATE_VALUE;
			}

			if (pNode->_data > data)
			{
				if (pNode->_pLeft == _pNil)
				{
					Node* pNewNode = new Node(data, _pNil);
					pNode->_pLeft = pNewNode;
					pNewNode->_pParent = pNode;
					BalanceAfterInsert(pNewNode);
					_size++;
					//printf("==================\n");
					//printf("INSERT -----> %d  \n", data);
					//printf("==================\n");
					return INSERT_SUCCESS;
				}
				else
				{
					pNode = pNode->_pLeft;
					continue;
				}
			}

			if (pNode->_data < data)
			{
				if (pNode->_pRight == _pNil)
				{
					Node* pNewNode = new Node(data, _pNil);
					pNode->_pRight = pNewNode;
					pNewNode->_pParent = pNode;
					BalanceAfterInsert(pNewNode);
					_size++;
					//printf("==================\n");
					//printf("INSERT -----> %d  \n", data);
					//printf("==================\n");
					return INSERT_SUCCESS;
				}
				else
				{
					pNode = pNode->_pRight;
					continue;
				}
			}
		}
		return INSERT_ERROR_UNKNOWN;
	}

public:
	int DeleteNode(DATA data)
	{
		Node* pNode = nullptr;
		bool bNodeLeft = false;

		if (_pRoot == _pNil)
			return DELETE_ERROR_TREE_IS_EMPTY;

		pNode = _pRoot;

		//search node
		while (pNode->_data != data)
		{
			if (pNode->_data > data)
			{
				if (pNode->_pLeft == _pNil)
				{
					return DELETE_ERROR_CANT_FIND;
				}
				else
				{
					pNode = pNode->_pLeft;
					bNodeLeft = true;
					continue;
				}
			}

			if (pNode->_data < data)
			{
				if (pNode->_pRight == _pNil)
				{
					return DELETE_ERROR_CANT_FIND;
				}
				else
				{
					pNode = pNode->_pRight;
					bNodeLeft = false;
					continue;
				}
			}
		}

		// Delete Input Data Node 
		if (pNode->_pLeft == _pNil &&
			pNode->_pRight == _pNil)
		{
			//printf("(0) ");

			if (pNode == _pRoot)
			{
				_pRoot = _pNil;
				delete(pNode);
			}
			else
			{
				if (bNodeLeft)
				{
					pNode->_pParent->_pLeft = _pNil;
				}
				else
				{
					pNode->_pParent->_pRight = _pNil;
				}

				COLOR color = pNode->_color;
				Node* pParent = pNode->_pParent;
				delete(pNode);

				if (color == BLACK)
					BalanceAfterDelete(_pNil, pParent);
			}

			_size--;
		}
		else if (pNode->_pLeft == _pNil)
		{

			if (pNode == _pRoot)
			{
				_pRoot = pNode->_pRight;
				delete(pNode);
			}
			else
			{
				if (bNodeLeft)
				{
					pNode->_pRight->_pParent = pNode->_pParent;
					pNode->_pParent->_pLeft = pNode->_pRight;
				}
				else
				{
					pNode->_pRight->_pParent = pNode->_pParent;
					pNode->_pParent->_pRight = pNode->_pRight;
				}

				Node* pChild = pNode->_pRight;
				COLOR color = pNode->_color;
				delete(pNode);

				if (color == BLACK)
					BalanceAfterDelete(pChild, pChild->_pParent);
			}

			_size--;
		}
		else if (pNode->_pRight == _pNil)
		{

			if (pNode == _pRoot)
			{
				_pRoot = pNode->_pLeft;
				delete(pNode);
			}
			else
			{
				if (bNodeLeft)
				{
					pNode->_pLeft->_pParent = pNode->_pParent;
					pNode->_pParent->_pLeft = pNode->_pLeft;
				}
				else
				{
					pNode->_pLeft->_pParent = pNode->_pParent;
					pNode->_pParent->_pRight = pNode->_pLeft;
				}

				Node* pChild = pNode->_pLeft;
				COLOR color = pNode->_color;
				delete(pNode);

				if (color == BLACK)
					BalanceAfterDelete(pChild, pChild->_pParent);
			}

			_size--;
		}
		else
		{			
			// 자식이 둘인 노드를 지우는 경우 
			// Delete Alternate Node 

			Node* pAlt = pNode->_pLeft;
			bool bAltLeft = true;


			while (pAlt->_pRight != _pNil)
			{
				pAlt = pAlt->_pRight;
				bAltLeft = false;
			}

			pNode->_data = pAlt->_data;

			if (bAltLeft)
			{
				pAlt->_pLeft->_pParent = pAlt->_pParent;
				pAlt->_pParent->_pLeft = pAlt->_pLeft;
			}
			else if (!bAltLeft)
			{
				pAlt->_pLeft->_pParent = pAlt->_pParent;
				pAlt->_pParent->_pRight = pAlt->_pLeft;
			}

			COLOR color = pAlt->_color;
			Node* pChild = pAlt->_pLeft;
			Node* pParent = pAlt->_pParent;
			delete(pAlt);

			if (color == BLACK)
				BalanceAfterDelete(pChild, pParent);

			_size--;
		}
		//printf("==================\n");
		//printf("DELETE -----> %d  \n", data);
		//printf("==================\n");
		return DELETE_SUCCESS;
	}

	void DeleteAllNode()
	{
		PostOrderDeleteNode(_pRoot);
		_pRoot = _pNil;
		_size = 0;
	}

	void PostOrderDeleteNode(Node* pNode)
	{
		if (pNode == _pNil) return;

		PostOrderDeleteNode(pNode->_pLeft);
		PostOrderDeleteNode(pNode->_pRight);
		delete(pNode);
	}

private:
	void BalanceAfterInsert(Node* pNode)
	{
		//R-R 발생했을 경우
		while (pNode->_pParent->_color != BLACK)
		{
			bool bParentLeft = false;
			if (pNode->_pParent == pNode->_pParent->_pParent->_pLeft)
				bParentLeft = true;

			// 삼촌이 R -> 색상반전
			if (bParentLeft
				&& pNode->_pParent->_pParent->_pRight->_color == RED)
			{
				pNode->_pParent->_color = BLACK;
				pNode->_pParent->_pParent->_pRight->_color = BLACK;

				if (pNode->_pParent->_pParent == _pRoot)
				{
					break;
				}
				else
				{
					pNode->_pParent->_pParent->_color = RED;
					pNode = pNode->_pParent->_pParent;
					continue;
				}
			}
			else if (!bParentLeft
				&& pNode->_pParent->_pParent->_pLeft->_color == RED)
			{
				pNode->_pParent->_color = BLACK;
				pNode->_pParent->_pParent->_pLeft->_color = BLACK;

				if (pNode->_pParent->_pParent == _pRoot)
				{
					break;
				}
				else
				{
					pNode->_pParent->_pParent->_color = RED;
					pNode = pNode->_pParent->_pParent;
					continue;
				}
			}
			else if (bParentLeft
				&& pNode->_pParent->_pParent->_pRight->_color == BLACK)
			{
				// LR case -> 이중 회전
				if (pNode == pNode->_pParent->_pRight)
				{
					RotateLeft(pNode->_pParent);
					pNode = pNode->_pLeft;
				}

				// LL case -> 싱글 회전
				pNode->_pParent->_color = BLACK;
				pNode->_pParent->_pParent->_color = RED;
				RotateRight(pNode->_pParent->_pParent);
				break;
			}
			else if (!bParentLeft
				&& pNode->_pParent->_pParent->_pLeft->_color == BLACK)
			{
				// RL case -> 이중 회전
				if (pNode == pNode->_pParent->_pLeft)
				{
					RotateRight(pNode->_pParent);
					pNode = pNode->_pRight;
				}

				// RR case -> 싱글 회전 
				pNode->_pParent->_color = BLACK;
				pNode->_pParent->_pParent->_color = RED;
				RotateLeft(pNode->_pParent->_pParent);
				break;
			}
		}
	}

	void BalanceAfterDelete(Node* pNode, Node* pParent)
	{
		while (pNode->_color != RED)
		{
			bool bNodeLeft = false;
			Node* pSibling = nullptr;
			if (pNode == pParent->_pLeft)
			{
				bNodeLeft = true;
				pSibling = pParent->_pRight;
			}
			else
			{
				pSibling = pParent->_pLeft;
			}

			if (pSibling->_color == RED)
			{
				pSibling->_color = BLACK;
				if (bNodeLeft)
					RotateLeft(pParent);
				else
					RotateRight(pParent);
				pParent->_color = RED;
				continue;
			}
			else if (pSibling->_color == BLACK &&
				pSibling->_pLeft->_color == BLACK &&
				pSibling->_pRight->_color == BLACK)
			{
				pSibling->_color = RED;
				if (pParent == _pRoot)
					return;
				pNode = pParent;
				pParent = pParent->_pParent;
				continue;
			}

			if (bNodeLeft &&
				pSibling->_color == BLACK &&
				pSibling->_pLeft->_color == RED &&
				pSibling->_pRight->_color == BLACK)
			{
				pSibling->_pLeft->_color = BLACK;
				pSibling->_color = RED;
				RotateRight(pSibling);
			}
			else if (!bNodeLeft &&
				pSibling->_color == BLACK &&
				pSibling->_pLeft->_color == BLACK &&
				pSibling->_pRight->_color == RED)
			{
				pSibling->_pRight->_color = BLACK;
				pSibling->_color = RED;
				RotateLeft(pSibling);
			}

			if (bNodeLeft &&
				pSibling->_color == BLACK &&
				pSibling->_pRight->_color == RED)
			{
				pSibling->_color = pParent->_color;
				pParent->_color = BLACK;
				pSibling->_pRight->_color = BLACK;
				RotateLeft(pParent);
				return;
			}

			else if (!bNodeLeft &&
				pSibling->_color == BLACK &&
				pSibling->_pLeft->_color == RED)
			{
				pSibling->_color = pParent->_color;
				pParent->_color = BLACK;
				pSibling->_pLeft->_color = BLACK;
				RotateRight(pParent);
				return;
			}
		}

		pNode->_color = BLACK;
		return;
	}

	void RotateLeft(Node* pNode)
	{
		if (pNode->_pRight == _pNil)
		{
			printf("Left Error: %d\n", pNode->_data);
			return;
		}

		if (pNode == _pRoot)
		{
			_pRoot = pNode->_pRight;
			pNode->_pRight->_color = BLACK;
		}
		else
		{
			if (pNode == pNode->_pParent->_pRight)
				pNode->_pParent->_pRight = pNode->_pRight;
			else if (pNode == pNode->_pParent->_pLeft)
				pNode->_pParent->_pLeft = pNode->_pRight; // G -> M
		}

		Node* pGrandChild = pNode->_pRight->_pLeft; // C
		pNode->_pRight->_pLeft = pNode; // M -> P
		pNode->_pRight->_pParent = pNode->_pParent; // M -> G
		pNode->_pParent = pNode->_pRight; // P -> M
		pNode->_pRight = pGrandChild; // P -> C
		pGrandChild->_pParent = pNode; // C -> P
	}

	void RotateRight(Node* pNode)
	{
		if (pNode->_pLeft == _pNil)
		{
			printf("Right Error: %d\n", pNode->_data);
			return;
		}

		if (pNode == _pRoot)
		{
			_pRoot = pNode->_pLeft;
			pNode->_pLeft->_color = BLACK;
		}
		else
		{
			if (pNode == pNode->_pParent->_pRight)
				pNode->_pParent->_pRight = pNode->_pLeft;
			else if (pNode == pNode->_pParent->_pLeft)
				pNode->_pParent->_pLeft = pNode->_pLeft;
		}

		Node* pGrandChild = pNode->_pLeft->_pRight;
		pNode->_pLeft->_pRight = pNode;
		pNode->_pLeft->_pParent = pNode->_pParent;
		pNode->_pParent = pNode->_pLeft;
		pNode->_pLeft = pGrandChild;
		pGrandChild->_pParent = pNode;
	}

public:
	bool SearchNode(DATA data)
	{
		if (_pRoot == _pNil)
		{
			//printf("현재 비어있는 트리입니다.\n");
			return false;
		}

		Node* pNode = _pRoot;

		while (pNode->_data != data)
		{
			if (pNode->_data > data)
			{
				if (pNode->_pLeft == _pNil)
				{
					//printf("존재하지 않는 값입니다: %d\n", data);
					return false;
				}
				else
				{
					pNode = pNode->_pLeft;
					continue;
				}
			}

			if (pNode->_data < data)
			{
				if (pNode->_pRight == _pNil)
				{
					//printf("존재하지 않는 값입니다: %d\n", data);
					return false;
				}
				else
				{
					pNode = pNode->_pRight;
					continue;
				}
			}
		}

		//printf("Success to Search %d!\n", data);
		return true;
	}

public:
	void PrintAllNode()
	{
		InOrderPrintNode(_pRoot);
		printf("\n");
	}

	int CheckAllPath(bool bForTest)
	{
		int leafIdx = 0;
		int black = 0;
		int red = 0;
		int pathBlackCnt = -1;

		int ret = PreOrderCheckPath(_pRoot, 
			leafIdx, black, red, 
			pathBlackCnt, BLACK, bForTest);
		return ret;
	}

private:
	void InOrderPrintNode(Node* pNode)
	{
		if (pNode == _pNil) return;

		InOrderPrintNode(pNode->_pLeft);
		printf("%d", pNode->_data);
		InOrderPrintNode(pNode->_pRight);
	}

	int PreOrderCheckPath(Node* pNode, 
		int& leafIdx, int black, int red, 
		int& pathBlackCnt, COLOR prevColor, bool bForTest)
	{
		if (pNode == _pNil)
		{
			black++;
			if (pathBlackCnt == -1)
			{
				pathBlackCnt = black;
			}
			else if (black != pathBlackCnt)
			{
				    printf("Black height mismatch detected: %d (current) vs %d (expected)\n", black, pathBlackCnt);
					return TEST_ERROR_UNBALANCED;
			}
			if (!bForTest)
				printf("Leaf %d: black-%d, red-%d\n", leafIdx, black, red);
			leafIdx++;
			return TEST_SUCCESS;
		}

		if (pNode->_color == BLACK)
		{
			black++;
		}
		else if (pNode->_color == RED)
		{
			red++;
			if (prevColor == RED)
			{
				printf("Double Red! leaf idx %d, data %d\n", leafIdx, pNode->_data);
				return TEST_ERROR_DOUBLE_RED;
			}
		}

		int ret;
		ret = PreOrderCheckPath(pNode->_pLeft, leafIdx, black, red, pathBlackCnt, pNode->_color, bForTest);
		if (ret != TEST_SUCCESS) return ret;
		ret = PreOrderCheckPath(pNode->_pRight, leafIdx, black, red, pathBlackCnt, pNode->_color, bForTest);
		if (ret != TEST_SUCCESS) return ret;
		return TEST_SUCCESS;
	}

public:
	int GetTreeSize()
	{
		return _size;
	}

	void GetAllNode(DATA* dataArray)
	{
		int index = 0;
		InOrderGetNode(_pRoot, dataArray, &index);
	}

private:
	void InOrderGetNode(Node* pNode, DATA* dataArray, int* index)
	{
		if (pNode == _pNil) return;

		InOrderGetNode(pNode->_pLeft, dataArray, index);
		dataArray[*index] = pNode->_data;
		(*index)++;
		InOrderGetNode(pNode->_pRight, dataArray, index);
	}

public:
	void DrawAllNode(HDC hdc, HPEN black, HPEN red, HBRUSH hBlackBrush, HBRUSH hRedBrush, int xPad, int yPad)
	{
		HPEN hPen = (HPEN)SelectObject(hdc, black);
		HBRUSH hBrush = (HBRUSH)SelectObject(hdc, hBlackBrush);

		DrawNode(hdc, red, hRedBrush, _pRoot,
			X_PIVOT, Y_PIVOT, X_PIVOT, Y_PIVOT,
			xPad, yPad, 0, 0, 0);
		SelectObject(hdc, hPen);
		SelectObject(hdc, hBrush);
	}

private:
	void DrawNode(HDC hdc, HPEN hRedPen, HBRUSH hRedBrush,
		Node* pNode, int prevXPos, int prevYPos,
		int xPos, int yPos, int xPad, int yPad,
		int left, int right, int depth)
	{
		MoveToEx(hdc, prevXPos + xPad, prevYPos + yPad, NULL);
		LineTo(hdc, xPos + xPad, yPos + yPad);

		WCHAR text[TEXT_LEN];
		if (pNode == _pNil)
		{
			wcscpy_s(text, TEXT_LEN, L"nil");
		}
		else
		{
			_itow_s(pNode->_data, text, TEXT_LEN, 10);
		}

		if (pNode->_color == RED)
		{
			HPEN hPen = (HPEN)SelectObject(hdc, hRedPen);
			HBRUSH hBrush = (HBRUSH)SelectObject(hdc, hRedBrush);

			Ellipse(hdc,
				xPos - RADIUS + xPad,
				yPos - RADIUS + yPad,
				xPos + RADIUS + xPad,
				yPos + RADIUS + yPad);

			SelectObject(hdc, hPen);
			SelectObject(hdc, hBrush);
		}
		else
		{
			Ellipse(hdc,
				xPos - RADIUS + xPad,
				yPos - RADIUS + yPad,
				xPos + RADIUS + xPad,
				yPos + RADIUS + yPad);
		}

		TextOutW(hdc,
			xPos - TEXT_PAD + xPad,
			yPos - TEXT_PAD + yPad,
			text, wcslen(text));

		if (pNode == _pNil) return;


		depth++;

		int prevXGap = abs(prevXPos - xPos);
		if (pNode == _pRoot)
			prevXGap = xPos;

		left++;
		DrawNode(hdc, hRedPen, hRedBrush, pNode->_pLeft, xPos, yPos,
			xPos - (prevXGap / 2), yPos + Y_GAP,
			xPad, yPad, left, right, depth);
		left--;

		right++;
		DrawNode(hdc, hRedPen, hRedBrush, pNode->_pRight, xPos, yPos,
			xPos + (prevXGap / 2), yPos + Y_GAP,
			xPad, yPad, left, right, depth);
		right--;

		depth--;
		return;
	}
};
