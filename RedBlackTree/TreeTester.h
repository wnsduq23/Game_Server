#pragma once
#include <windows.h>
#include <stdio.h>
#include <set>

//#include "Profile.h"
#include "RedBlackTree.h"
#include "BinaryTree.h"

using namespace std;
#define MOVE 15

class TreeTester
{
public:
	TreeTester();
	~TreeTester();

public:
	void DrawTree(HDC hdc);
	void PrintMenu();

public:
	void MoveRight() { _iCurX += MOVE; }
	void MoveLeft() { _iCurX -= MOVE; }
	void MoveUp() { _iCurY -= MOVE; }
	void MoveDown() { _iCurY += MOVE; }

public:
	void DrawTree();
	void InsertNode();
	void InsertRandomNode();
	void DeleteNode();
	void DeleteRandomNode();
	void SearchNode();

	void CheckBlackBalance();
	void CheckAscending();
	void AutoTest();

private:
	bool GetTreeDataForTest(set<int>& testSet);
	bool InsertForTest(int count, set<int>& testSet);
	bool DeleteForTest(int count, set<int>& testSet);

private:
	bool	 _bDrawRedBlackTree = true;
	int		_iCurX = 0;
	int		_iCurY = 0;
	HPEN	_hBlackPen;
	HPEN	_hRedPen;
	HBRUSH	_hRedBrush;
	HBRUSH	_hBlackBrush;

	RedBlackTree<int> _RedBlackTree;
	BinaryTree<int> _BinaryTree;
};

