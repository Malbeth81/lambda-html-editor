#ifndef TOOLBAR_H
#define	TOOLBAR_H

#define _WIN32_IE 0x0300
#include <windows.h>
#include <commctrl.h>

#define NUM_BUTTONS 26
#define ICON_SIZE 16
#define TOOLBAR_HEIGHT ICON_SIZE+6+4

HWND CreateToolbar(HWND Parent);

LRESULT CALLBACK ToolbarWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#endif

