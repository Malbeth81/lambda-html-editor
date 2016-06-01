#include <windows.h>
#include "../tools.h"

LONG OldEditTextWndProc = 0;

void UpdateEditValue(HWND hWnd, int Increment)
{
  char* Str = GetWindowText(hWnd);
  int Index = Max(1, atoi(Str)+Increment);
  delete[] Str;
  Str = IntToStr(Index);
  SetWindowText(hWnd, Str);
  delete[] Str;
}

LRESULT CALLBACK ScrollEditWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_VSCROLL:
    {
      if (LOWORD(wParam) == SB_LINEDOWN)
        UpdateEditValue(hWnd, -1);
      else if (LOWORD(wParam) == SB_LINEUP)
        UpdateEditValue(hWnd, 1);
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      if ((short)HIWORD(wParam) < 0)
        UpdateEditValue(hWnd, -1);
      else
        UpdateEditValue(hWnd, 1);
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)OldEditTextWndProc, hWnd, Msg, wParam, lParam);
}

void MakeEditScrollable(HWND hWnd)
{
  LONG Result = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&ScrollEditWndProc);
  if (OldEditTextWndProc == 0)
    OldEditTextWndProc = Result;
}
