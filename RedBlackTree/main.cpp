#include "main.h"
#include "TreeTester.h"
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console" )

TreeTester* g_pTreeTester;

HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;
HDC		g_hMemDC;
RECT	g_MemDCRect;


int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	WNDCLASSEX	wcex;
	HWND		hWnd;
	MSG			msg;

	wcex.cbSize = sizeof(WNDCLASSEX);				// 구조체 크기(구조체 크기 이상을 사용할때의 문제를 방지)
	wcex.style = CS_HREDRAW | CS_VREDRAW;			// 창의 크기가 변경될 때, 창의 콘텐츠를 다시 그릴 지 여부를 결정
	wcex.lpfnWndProc = WndProc;						// 메시지 처리 함수 등록
	wcex.cbClsExtra = 0;							// 클래스 추가 메모리 크기
	wcex.cbWndExtra = 0;							// 윈도우 추가 메모리 크기
	wcex.hInstance = hInstance;						// 프로그램 정보
	wcex.hIcon = NULL;								// 아이콘
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);	// 커서 형태
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// 윈도우 바탕색
	wcex.lpszClassName = L"RedBlackTree";			// 클래스 이름으로 윈도우 생성
	wcex.hIconSm = NULL;
	wcex.lpszMenuName = NULL;						// 윈도우 메뉴
	if (!RegisterClassEx(&wcex)) 
	{
		MessageBox(NULL, L"Window Class Registration Failed!", L"Error", MB_OK);
		return (1);
	}

	hWnd = CreateWindowW(L"RedBlackTree",		// 윈도우 클래스 이름 
		L"RB_TREE",									// 윈도우 제목
		WS_OVERLAPPEDWINDOW,						// 윈도우 스타일
		CW_USEDEFAULT,								// 윈도우 x 좌표
		CW_USEDEFAULT,								// 윈도우 y 좌표
		CW_USEDEFAULT,								// 윈도우 너비
		CW_USEDEFAULT,								// 윈도우 높이
		nullptr,									// 부모 윈도우 핸들
		nullptr,									// 메뉴 핸들
		hInstance,									// app의 인스턴스 핸들
		nullptr);									// 추가 매개변수

	if (!hWnd)
	{
		MessageBox(NULL, L"Window Created Failed!", L"ERROR", MB_OK);
		return (1);
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	switch (message)
	{
	case WM_CREATE:
		g_pTreeTester = new TreeTester;

        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_UP:
			g_pTreeTester->MoveUp();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case VK_DOWN:
			g_pTreeTester->MoveDown();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case VK_LEFT:
			g_pTreeTester->MoveLeft();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case VK_RIGHT:
			g_pTreeTester->MoveRight();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case VK_NUMPAD1:
		case VK_NUMBER1:
			g_pTreeTester->InsertNode();
			InvalidateRect(hWnd, NULL, TRUE);
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD2:
		case VK_NUMBER2:
			g_pTreeTester->InsertRandomNode();
			InvalidateRect(hWnd, NULL, TRUE);
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD3:
		case VK_NUMBER3:
			g_pTreeTester->DeleteNode();
			InvalidateRect(hWnd, NULL, TRUE);
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD4:
		case VK_NUMBER4:
			g_pTreeTester->DeleteRandomNode();
			InvalidateRect(hWnd, NULL, TRUE);
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD5:
		case VK_NUMBER5:
			g_pTreeTester->SearchNode();
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD6:
		case VK_NUMBER6:
			g_pTreeTester->CheckAscending();
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD7:
		case VK_NUMBER7:
			g_pTreeTester->CheckBlackBalance();
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD8:
		case VK_NUMBER8:
			g_pTreeTester->AutoTest();
			InvalidateRect(hWnd, NULL, TRUE);
			g_pTreeTester->PrintMenu();
			break;

		case VK_NUMPAD9:
		case VK_NUMBER9:
			break;

		case VK_NUMPAD0:
		case VK_NUMBER0:
			system("cls");
			g_pTreeTester->PrintMenu();
			break;

		default:
			break;
		}
	}
	break;
	case WM_PAINT:

		PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);
		g_pTreeTester->DrawTree();

		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, g_MemDCRect.right,
			g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:

		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);
		delete(g_pTreeTester);
		PostQuitMessage(0);
		break;

	case WM_SIZE:

		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);

		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &g_MemDCRect);
		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);

		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
		PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);

		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return (0);
}