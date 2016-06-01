#include <windows.h>
#include "../main.h"

#define GripCount 8

LONG OldSplitterWndProc;
int SplitterPos = 200;
bool HideSplitter = false;
HCURSOR SplitterCursor = LoadCursor(NULL, IDC_SIZEWE);

LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static int XPos;
  switch (Msg)
  {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      SetCursor(SplitterCursor);
      SetCapture(hWnd);
      XPos = (short)LOWORD(lParam);
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      SetCursor(SplitterCursor);
      ReleaseCapture();
      return 0;
    }
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
      SetCursor(SplitterCursor);
      Explorer->SetHide(!Explorer->GetHide());
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      SetCursor(SplitterCursor);
      if (Explorer->IsVisible() && GetCapture() == hWnd)
      {
        /* Obtain parent window size */
        RECT R = GetClientRect();
        /* Obtain cursor pos */
        POINT P;
        GetCursorPos(&P);
        ScreenToClient(GetParent(hWnd), &P);
        SplitterPos = Max(100, Min(P.x-XPos, R.right-5-100));
        AdjustSize();
      }
      else if (Explorer->GetAutoHide())
        Explorer->SetHide(false);
      return 0;
    }
    case WM_NCHITTEST:
    {
      return HTCLIENT;
    }
    case WM_PAINT:
    {
      if (GetUpdateRect(hWnd, NULL, 0) != 0)
      {
        PAINTSTRUCT PS;
        HDC DC = BeginPaint(hWnd, &PS);
        if (DC != NULL)
        {
          RECT R;
          GetClientRect(hWnd, &R);
          /* Paints background */
          HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE)));
          HBRUSH OldBrush = (HBRUSH)SelectObject(DC, CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));
          Rectangle(DC, R.left, R.top, R.right, R.bottom);
          DeleteObject(SelectObject(DC, OldPen));
          DeleteObject(SelectObject(DC, OldBrush));
          /* Paints gripper */
          R.top = (R.bottom-R.top-5*GripCount-2)/2;
          R.left = (R.right-R.left-3)/2;
          R.bottom = R.top+3;
          R.right = R.left+3;
          for (int i = 0; i < GripCount; i++)
          {
            DrawEdge(DC, &R, BDR_SUNKENOUTER, BF_RECT);
            OffsetRect(&R, 0, 5);
          }
          EndPaint(hWnd, &PS);
        }
      }
      return 0;
    }
    case WM_SIZE:
    {
      InvalidateRect(hWnd, NULL, 0);
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)OldSplitterWndProc, hWnd, Msg, wParam, lParam);
}

HWND CreateSplitter(HWND Parent)
{
  HWND Handle = CreateWindow("STATIC", NULL,  WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
    0, 0, 0, 0, Parent, NULL, GetModuleHandle(NULL), NULL);
  OldSplitterWndProc = SetWindowLong(Handle, GWL_WNDPROC, (LONG)&SplitterWndProc);
  return Handle;
}
