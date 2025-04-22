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

	wcex.cbSize = sizeof(WNDCLASSEX);				// ����ü ũ��(����ü ũ�� �̻��� ����Ҷ��� ������ ����)
	wcex.style = CS_HREDRAW | CS_VREDRAW;			// â�� ũ�Ⱑ ����� ��, â�� �������� �ٽ� �׸� �� ���θ� ����
	wcex.lpfnWndProc = WndProc;						// �޽��� ó�� �Լ� ���
	wcex.cbClsExtra = 0;							// Ŭ���� �߰� �޸� ũ��
	wcex.cbWndExtra = 0;							// ������ �߰� �޸� ũ��
	wcex.hInstance = hInstance;						// ���α׷� ����
	wcex.hIcon = NULL;								// ������
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);	// Ŀ�� ����
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// ������ ������
	wcex.lpszClassName = L"RedBlackTree";			// Ŭ���� �̸����� ������ ����
	wcex.hIconSm = NULL;
	wcex.lpszMenuName = NULL;						// ������ �޴�
	if (!RegisterClassEx(&wcex)) 
	{
		MessageBox(NULL, L"Window Class Registration Failed!", L"Error", MB_OK);
		return (1);
	}

	hWnd = CreateWindowW(L"RedBlackTree",		// ������ Ŭ���� �̸� 
		L"RB_TREE",									// ������ ����
		WS_OVERLAPPEDWINDOW,						// ������ ��Ÿ��
		CW_USEDEFAULT,								// ������ x ��ǥ
		CW_USEDEFAULT,								// ������ y ��ǥ
		CW_USEDEFAULT,								// ������ �ʺ�
		CW_USEDEFAULT,								// ������ ����
		nullptr,									// �θ� ������ �ڵ�
		nullptr,									// �޴� �ڵ�
		hInstance,									// app�� �ν��Ͻ� �ڵ�
		nullptr);									// �߰� �Ű�����

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