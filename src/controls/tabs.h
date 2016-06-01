#ifndef TABS_H
#define	TABS_H

#include <windows.h>
#include <commctrl.h>

#define WM_SELECTDOCUMENT WM_USER+96

typedef void (__stdcall *HDOCMOVEDPROC)(int Index, int NewIndex);

extern HDOCMOVEDPROC DocumentMovedProc;
    
void AddTab(HWND Tabs, const char* Caption);
HWND CreateTabs(HWND Parent);
void EditTab(HWND Tabs, int Index, char* Caption);
void MoveTab(HWND Tabs, int FromIndex, int ToIndex);
void RemoveTab(HWND Tabs, int Index);
void SetActiveTab(HWND Tabs, int Index);
LRESULT CALLBACK TabWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#endif
