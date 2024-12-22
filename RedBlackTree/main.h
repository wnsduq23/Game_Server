#pragma once

#include "BinaryTree.h"
#include <windows.h>
#include <time.h>
#include <stdio.h>

/*=======================
*	Global Variable
*=======================*/

extern HPEN		g_hPen;
extern HBITMAP	g_hMemDCBitmap;
extern HBITMAP	g_hMemDCBitmap_old;
extern HDC		g_hMemDC;
extern RECT		g_MemDCRect;

/*=======================
*			DEFINE
*=======================*/
#define VK_NUMBER1 0x31
#define VK_NUMBER2 0x32
#define VK_NUMBER3 0x33
#define VK_NUMBER4 0x34
#define VK_NUMBER5 0x35
#define VK_NUMBER6 0x36
#define VK_NUMBER7 0x37
#define VK_NUMBER8 0x38
#define VK_NUMBER9 0x39
#define VK_NUMBER0 0x30

/*=======================
*		FUNCTION
*=======================*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
