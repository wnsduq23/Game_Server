#include "framework.h"
#include "windowsx.h"
#include "Network.h"
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console" )

#define MAX_LOADSTRING 100
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL g_bClick;
int iOldX = 0;
int iOldY = 0;
int iCurX = 0;
int iCurY = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    //UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg;
    HWND hWnd;
    WNDCLASSEXW wcex;

    // window 按眉 积己 棺 技泼
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"Window";
    wcex.hIconSm = 0;
    RegisterClassEx(&wcex);

    hWnd = CreateWindowW(L"Window",
        L"Net Drawing !!!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0,
        nullptr, nullptr,
        hInstance, nullptr);

    if (!hWnd)
        return FALSE;

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
    switch (message)
    {
    case WM_CREATE:
        g_hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
        if (!InitialSocket(hWnd))
            PostQuitMessage(0);
        break;

    case WM_MY_SOCKET:
        ProcessSocketMessage(hWnd, message, wParam, lParam);
        break;

    case WM_LBUTTONDOWN:
        g_bClick = true;
        break;

    case WM_LBUTTONUP:
        g_bClick = false;
        break;

    case WM_MOUSEMOVE:

        iCurX = GET_X_LPARAM(lParam);
        iCurY = GET_Y_LPARAM(lParam);

        if (g_bClick)
        {
            DRAW_PACKET drawPacket;
            drawPacket.iStartX = iOldX;
            drawPacket.iStartY = iOldY;
            drawPacket.iEndX = iCurX;
            drawPacket.iEndY = iCurY;

            EnqueueSendData((char*)&drawPacket, (unsigned short)sizeof(DRAW_PACKET));
            SendUnicast();

        }

        iOldX = iCurX;
        iOldY = iCurY;
        break;

    case WM_DESTROY:
        DeleteObject(g_hPen);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}