#include <windows.h>
#include "../resource.h"
#include "../main.h"

TRACKMOUSEEVENT MouseEvent;

LONG DefaultStaticWndProc;

HCURSOR LinkCursor = LoadCursor(NULL, MAKEINTRESOURCE(32649));
LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  {
    char* Str = GetWindowText(hWnd);
    ShellExecute(NULL, "open", Str, NULL, NULL, SW_SHOW);
    delete[] Str;
  }
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_MOUSEMOVE:
    SetCursor(LinkCursor);
    return FALSE;
  }
  return CallWindowProc((WNDPROC)DefaultStaticWndProc, hWnd, Msg, wParam, lParam);
}

INT_PTR CALLBACK AboutDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  HDC DC;
  switch (uMsg)
  {
  case WM_COMMAND:        /* Process control messages */
    switch (LOWORD(wParam))
    {
    case IDCANCEL:      // Generic message sent by IsDialogMessage for Esc key
    case ID_OKBTN:
      SendMessage(hDlg, WM_CLOSE, 0, 0);
      break;
    }
    return TRUE;
  case WM_CLOSE:
    EndDialog(hDlg, wParam);
    return TRUE;
  case WM_INITDIALOG:
    DefaultStaticWndProc = SetWindowLong(GetDlgItem(hDlg, ID_ABOUTLINK), GWL_WNDPROC, (LONG)&StaticWndProc);
    DC = GetDC(hDlg);
    SendDlgItemMessage(hDlg, ID_ABOUTLINK, WM_SETFONT, (WPARAM)EasyCreateFont(DC, "MS Shell Dlg", 8, fsUnderline), 0);
    ReleaseDC(hDlg, DC);
    return TRUE;
  default:
    return FALSE;
  }
}

void ShowAboutDialog()
{
  DialogBox(hInstance, MAKEINTRESOURCE(ID_ABOUTDIALOG), hWindow, (DLGPROC)AboutDialogProc);
}
