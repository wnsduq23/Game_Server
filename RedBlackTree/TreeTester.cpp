#include "TreeTester.h"
#include "main.h"
#include <list>

//#define BINARY_TEST
#define REDBLACK_TEST

TreeTester::TreeTester()
{
    printf("Start TreeTester!!!\n");
    PrintMenu();

	srand(20);
	_hBlackPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	_hRedPen = CreatePen(PS_SOLID, 1, RGB(200, 0, 0));

	_hBlackBrush = CreateSolidBrush(RGB(100, 100, 100));
	_hRedBrush = CreateSolidBrush(RGB(200, 0, 0));
}

void TreeTester::DrawTree()
{
#ifdef BINARY_TEST
    _BinaryTree.DrawAllNode(g_hMemDC, _iCurX, _iCurY);
#endif
#ifdef REDBLACK_TEST
    _RedBlackTree.DrawAllNode(g_hMemDC, _hBlackPen, _hRedPen, _hBlackBrush, _hRedBrush, _iCurX, _iCurY);
#endif
}

void TreeTester::SearchNode()
{
    // INPUT
	char nodeInput[INPUT_LEN] = { '\0', };
	printf("Enter Number to Search\n");
	fgets(nodeInput, INPUT_LEN, stdin);
	int num = atoi(nodeInput);

    // LOGIC
#ifdef BINARY_TEST
    _BinaryTree.SearchNode(node);
#endif
#ifdef REDBLACK_TEST
   bool ret =  _RedBlackTree.SearchNode(num);
#endif
   if (ret)
   {
	   printf("Success to Search %d\n", num);
   }
   else
   {
	   printf("There is no %d\n", num);
   }

}

void TreeTester::InsertNode()
{

    // INPUT
	char nodeInput[INPUT_LEN] = { '\0', };
	printf("Enter Number to Insert\n");
	fgets(nodeInput, INPUT_LEN, stdin);
	int node = atoi(nodeInput);

    // LOGIC
#ifdef BINARY_TEST
	_BinaryTree.InsertNode(node);
#endif
#ifdef REDBLACK_TEST
	int ret = _RedBlackTree.InsertNode(node);
#endif
	if (ret == INSERT_ERROR_UNKNOWN)
	{
		printf("Insert Unknown Error! Cur Data: %d\n", node);
	}
	else if (ret == INSERT_ERROR_TREE_IS_FULL)
	{
		printf("Tree is full!\n"
			"Cur Data: %d\n",
			node);
	}
	else if (ret == INSERT_ERROR_DUPLICATE_VALUE)
	{
		printf("Fail to Insert! %d : Duplicate Data\n", node);
	}
}

void TreeTester::InsertRandomNode()
{

    // INPUT
	char nodeInput[INPUT_LEN] = { '\0', };
	printf("Enter Node Count to Insert (MAX: %d)\n", INT_MAX);
	fgets(nodeInput, INPUT_LEN, stdin);
	int node = atoi(nodeInput);

    if (node > INT_MAX || node < 0)
    {
        printf("Wrong Value!\n");
        return;
    }

	printf("Requested Count: %d\n", node);

    // LOGIC
    list<int> duplist;
    int ret;
    for (int i = 0; i < node; i++)
    {
        int rand1 = rand();
        int rand2 = rand();
        rand1 = rand1 << 7;
        rand1 |= rand2;
        printf("Insert ==========> %d\n", rand1);

#ifdef REDBLACK_TEST
		ret = _RedBlackTree.InsertNode(rand1);
#endif

#ifdef BINART_TEST
		_BinaryTree.InsertNode(rand1);
#endif
        if (ret == INSERT_ERROR_UNKNOWN)
        {
            printf("Insert Unknown Error! Cur Data: %d\n", rand1);
            break;
        }
        else if (ret == INSERT_ERROR_TREE_IS_FULL)
        {
			printf("Tree is full!\n"
				"Cur Data: %d, Success Count: %d, Fail Count: %d\n",
				rand1, i, node - i + 1);
            break;
        }
        else if (ret == INSERT_ERROR_DUPLICATE_VALUE)
        {
            duplist.push_back(rand1);
            printf("Fail to Insert! %d : Duplicate Data\n", rand1);
        }
    }
   printf("Duplicate Data Count: %llu\n", duplist.size());
}

void TreeTester::DeleteNode()
{
    // INPUT
	char nodeInput[INPUT_LEN] = { '\0', };
	printf("Enter Number to Delete\n");
	fgets(nodeInput, INPUT_LEN, stdin);
	int node = atoi(nodeInput);

	// LOGIC
#ifdef BINARY_TEST
	_BinaryTree.DeleteNode(node);
#endif
#ifdef REDBLACK_TEST
	int ret = _RedBlackTree.DeleteNode(node);
#endif
       if (ret == DELETE_ERROR_CANT_FIND)
        {
            printf("ERROR! CAN'T FIND %d\n", node);
        }
        else if (ret == DELETE_ERROR_UNKNOWN)
        {
            printf("Unknown Error!\n");
            printf("Cur Data: %d\n",
                node);
        }
}

void TreeTester::DeleteRandomNode()
{
    // INPUT
	char nodeInput[INPUT_LEN] = { '\0', };
#ifdef BINARY_TEST
	printf("Enter Node Count to Delete (MAX: %d)\n", _BinaryTree.GetTreeSize());
#endif
#ifdef REDBLACK_TEST
	printf("Enter Node Count to Delete (MAX: %d)\n", _RedBlackTree.GetTreeSize());
#endif
	fgets(nodeInput, INPUT_LEN, stdin);
	int node = atoi(nodeInput);

#ifdef BINARY_TEST
	if (node > _BinaryTree.GetTreeSize() || node < 0)
    {
        printf("Wrong Value!\n");
        return;
    }
#endif
#ifdef REDBLACK_TEST
	if (node > _RedBlackTree.GetTreeSize() || node < 0)
    {
        printf("Wrong Value!\n");
        return;
    }
#endif

	printf("Requested Count: %d\n", node);

#ifdef BINARY_TEST
	int max = _BinaryTree.GetTreeSize();
	int* data = new int[max];
	_BinaryTree.GetAllNode(data);
#endif
#ifdef REDBLACK_TEST
	int max = _RedBlackTree.GetTreeSize();
	int* data = new int[max];
	_RedBlackTree.GetAllNode(data);
#endif

    // LOGIC
    list<int> cantFindList;
    int ret;
    if (node <= RAND_MAX)
    {
        for (int i = 0; i < node; i++)
        {
            int rand1 = (rand() % max);
            while (data[rand1] == -1)
            {
                rand1 = (rand() % max);
            }
#ifdef BINARY_TEST
			_BinaryTree.DeleteNode(data[rand1]);
#endif
#ifdef REDBLACK_TEST
			int ret = _RedBlackTree.DeleteNode(data[rand1]);
#endif
            data[rand1] = -1;

			if (ret == DELETE_ERROR_CANT_FIND)
			{
				cantFindList.push_back(rand1);
			}
			else if (ret == DELETE_ERROR_UNKNOWN)
			{
				printf("Unknown Error!\n"
					"Cur Data: %d, Success Count: %d, Fail Count: %d\n",
					rand1, i, node - i + 1);
				break;
			}
        }
    }
    else
    {
        for (int i = 0; i < node; i++)
        {
            int rand1 = rand();
            int rand2 = rand();
            rand1 = rand1 << 7;
            rand1 |= rand2;
            rand1 %= max;

            while (data[rand1] == INT_MAX)
            {
                rand1 = rand();
                rand2 = rand();
                rand1 = rand1 << 7;
                rand1 |= rand2;
                rand1 %= max;
            }

#ifdef BINARY_TEST
			_BinaryTree.DeleteNode(data[rand1]);
#endif
#ifdef REDBLACK_TEST
			int ret = _RedBlackTree.DeleteNode(data[rand1]);
#endif
			data[rand1] = -1;

			if (ret == DELETE_ERROR_CANT_FIND)
			{
				cantFindList.push_back(rand1);
			}
			else if (ret == DELETE_ERROR_UNKNOWN)
			{
				printf("Unknown Error!\n"
					"Cur Data: %d, Success Count: %d, Fail Count: %d\n",
					rand1, i, node - i + 1);
				break;
			}
        }
    }
    delete[] data;

	printf("Can't Find Data Count: %llu\n", cantFindList.size());
}

void TreeTester::CheckAscending()
{
#ifdef BINARY_TEST
	int max = _BinaryTree.GetTreeSize();
	int* data = new int[max];
	_BinaryTree.GetAllNode(data);
#endif
#ifdef REDBLACK_TEST
	int max = _RedBlackTree.GetTreeSize();
	int* data = new int[max];
	_RedBlackTree.GetAllNode(data);
#endif
	printf("\ntotal size: %d\n", max);
    for (int i = 0; i < max - 1; i++)
    {
        if (data[i] > data[i + 1])
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			// 텍스트 색상을 빨간색으로 설정
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("\n================\n");
            printf("ERROR DETECTED!!!\n");
            printf("================\n");
			printf("%d \n", data[i]);
            // 텍스트 색상을 기본 색상으로 복원
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			return;
        }
		printf("%d ", data[i]);
    }
	printf("%d \n", data[max - 1]);
    printf("=================\n");
    printf("FINISH ASCENDING CHECK!!\n");
    printf("=================\n");
}

void TreeTester::CheckBlackBalance()
{
    _RedBlackTree.CheckAllPath(NOT_FOR_TEST);
    printf("=================\n");
    printf("FINISH BLACK BALANCE CHECK!!\n");
    printf("=================\n");
}

void TreeTester::AutoTest()
{
	printf("Press Enter to Stop Test\n");
    _RedBlackTree.DeleteAllNode();
    set<int> testSet;

    int insertRand;
    int insertDataAcc = 0;
    int insertDataCnt = 0;

    int deleteRand;
    int deleteDataAcc = 0;
    int deleteDataCnt = 0;

    int idx = 0;

    while (1)
    {
        idx++;

        if (GetAsyncKeyState(VK_RETURN))
            break;

        insertRand = rand();
        if (!InsertForTest(insertRand, testSet)) break;
        if (!GetTreeDataForTest(testSet))  break;
        insertDataCnt += insertRand;

        deleteRand = rand();
        if (!DeleteForTest(deleteRand, testSet)) break;
        if (!GetTreeDataForTest(testSet))  break;
        deleteDataCnt += deleteRand;

        if (insertDataCnt > CHECKPOINT)
        {
            insertDataCnt -= CHECKPOINT;
            insertDataAcc++;
            printf("Insert Success: %d\n",
                insertDataAcc * CHECKPOINT + insertDataCnt);
        }

        if (deleteDataCnt > CHECKPOINT)
        {
            deleteDataCnt -= CHECKPOINT;
            deleteDataAcc++;
            printf("Delete Success: %d\n",
                deleteDataAcc * CHECKPOINT + deleteDataCnt);
        }
    }

    printf("\n<Test Result>\n"
        "Loop Count: %d\n"
        "Insert Success: %d\n"
        "Delete Success: %d\n\n",
        idx,
        insertDataAcc * CHECKPOINT + insertDataCnt,
        deleteDataAcc * CHECKPOINT + deleteDataCnt
    );
}

bool TreeTester::GetTreeDataForTest(set<int>& testSet)
{
    if (_RedBlackTree.CheckAllPath(FOR_TEST) != TEST_SUCCESS)
    {
        return false;
    }

    int treeSize = _RedBlackTree.GetTreeSize();
    int* data = new int[treeSize];
    _RedBlackTree.GetAllNode(data);

    // 노드 개수가 같은 지
    if (treeSize != testSet.size())
    {
        printf("data count is different! tree: %d <-> test set: %llu\n",
            treeSize, testSet.size());
    }

    // 오름차순 정렬 확인 및 모든 노드가 있는 지 
    int idx = 0;
    set<int>::iterator iter = testSet.begin();
    for (; iter != testSet.end(); iter++) {

        if (data[idx] != *iter)
        {
            printf("data is different! tree: %d <-> test set: %d\n",
                data[idx], *iter);
            return false;
        }

        idx++;
    }

    return true;
}

bool TreeTester::InsertForTest(int count, set<int>& testSet)
{
    int ret;
    for (int i = 0; i < count; i++)
    {
        int rand1 = rand();
        int rand2 = rand();
        rand1 = rand1 << 7;
        rand1 |= rand2;

#ifdef REDBLACK_TEST
		ret = _RedBlackTree.InsertNode(rand1);
#endif

#ifdef BINART_TEST
		_BinaryTree.InsertNode(rand1);
#endif
        if (ret == INSERT_ERROR_UNKNOWN)
        {
            printf("Insert Unknown Error! Cur Data: %d\n", rand1);
            return false;
        }
        else if (ret == INSERT_ERROR_TREE_IS_FULL)
        {
            break;
        }
        testSet.insert(rand1);
    }
    return true;
}

bool TreeTester::DeleteForTest(int count, set<int>& testSet)
{
    int max = _RedBlackTree.GetTreeSize();
    int* data = new int[max];
    _RedBlackTree.GetAllNode(data);

    if (count > max) count = max;
    int ret;
    for (int i = 0; i < count; i++)
    {
        int rand1 = rand();
        int rand2 = rand();
        rand1 = rand1 << 7;
        rand1 |= rand2;
        rand1 %= max;

        while (data[rand1] == -1)
        {
            rand1 = rand();
            rand2 = rand();
            rand1 = rand1 << 7;
            rand1 |= rand2;
            rand1 %= max;
		}
#ifdef REDBLACK_TEST
		ret = _RedBlackTree.DeleteNode(data[rand1]);
#endif

#ifdef BINART_TEST
		_BinaryTree.DeleteNode(data[rand1]);
#endif

        int deleted = data[rand1];
        data[rand1] = -1;

        if (ret == DELETE_ERROR_UNKNOWN)
        {
            printf("Delete Unknown Error!: %d\n", rand1);
            delete[] data;
            return false;
        }

        testSet.erase(deleted);
    }

    delete[] data;
    return true;
}

void TreeTester::PrintMenu()
{
                printf(
                "\n1. Insert Node\n"
                "2. Insert Random Node\n"
                "3. Delete Node\n"
                "4. Delete Random Node\n"
                "5. Search Node\n"
                "6. Test1: Ascending Check \n"
                "7. Test2: Black Balance Check\n"
                "8. Auto Test (Test1 + Test2) \n"
                "0. Clear Console\n\n"
                "Choose number\n");
}