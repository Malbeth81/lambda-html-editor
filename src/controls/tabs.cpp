#include "tabs.h"
#include "../tools.h"

#define TAB_MAX_CHARS 32

LONG DefaultTabWndProc;

HDOCMOVEDPROC DocumentMovedProc = NULL;

// PRIVATE FUNCTIONS -----------------------------------------------------------

char* FormatCaption(const char* Caption)
{
  char* Str = new char[TAB_MAX_CHARS+1];
  strncpy(Str, Caption, TAB_MAX_CHARS);
  if (strlen(Caption) > TAB_MAX_CHARS)
  {
    Str[TAB_MAX_CHARS] = '.';
    Str[TAB_MAX_CHARS-1] = '.';
    Str[TAB_MAX_CHARS-2] = '.';
  }
  return Str;
}

int GetTabIndex(HWND hWnd, short x, short y)
{
  TC_HITTESTINFO HitTest;
  HitTest.pt.x = x;
  HitTest.pt.y = y;
  return TabCtrl_HitTest(hWnd, &HitTest);
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

void AddTab(HWND Tabs, const char* Caption)
{
  if (Tabs != NULL)
  {
    char* Str = FormatCaption(Caption);
    TCITEM TabItem;
    TabItem.mask = TCIF_TEXT | TCIF_IMAGE;
    TabItem.iImage = -1;
    TabItem.pszText = Str;
    TabCtrl_InsertItem(Tabs, TabCtrl_GetItemCount(Tabs), &TabItem);
    delete[] Str;
  }
}

HWND CreateTabs(HWND Parent)
{
  HWND Tabs = CreateWindowEx(WS_EX_ACCEPTFILES, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
    0, 0, 0, 0, Parent, NULL, GetModuleHandle(NULL), NULL);
  HDC DC = GetWindowDC(Tabs);
  SendMessage(Tabs, WM_SETFONT, (WPARAM)EasyCreateFont(DC, "MS Shell Dlg", 8, 0), 0);
  ReleaseDC(Tabs, DC);
  DefaultTabWndProc = SetWindowLong(Tabs, GWL_WNDPROC, (LONG)&TabWndProc);
  return Tabs;
}

void EditTab(HWND Tabs, int Index, char* Caption)
{
  if (Tabs != NULL)
  {
    char* Str = FormatCaption(Caption);
    TCITEM TabItem;
    TabItem.mask = TCIF_TEXT | TCIF_IMAGE;
    TabItem.iImage = -1;
    TabItem.pszText = Str;
    TabCtrl_SetItem(Tabs, Index, &TabItem);
    delete[] Str;
  }
}

void MoveTab(HWND Tabs, int FromIndex, int ToIndex)
{
  /* Retreive item information */
  TC_ITEM Item;
  Item.mask = TCIF_TEXT;
  Item.pszText = new char[MAX_PATH];
  Item.cchTextMax = MAX_PATH;
  TabCtrl_GetItem(Tabs, FromIndex, &Item);
  /* Move item */
  TabCtrl_DeleteItem(Tabs, FromIndex);
  TabCtrl_InsertItem(Tabs, ToIndex, &Item);
}

void RemoveTab(HWND Tabs, int Index)
{
  if (Tabs != NULL)
    TabCtrl_DeleteItem(Tabs, Index);
}

void SetActiveTab(HWND Tabs, int Index)
{
  if (Tabs != NULL)
    TabCtrl_SetCurSel(Tabs, Index);
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT CALLBACK TabWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static bool Dragging = false;
  static POINT Pos;
  switch (Msg)
  {
    case WM_DROPFILES:
    {
      return SendMessage(GetParent(hWnd), Msg, wParam, lParam);
    }
    case WM_LBUTTONDOWN:
    {
      if (GetCapture() != hWnd)
        SetCapture(hWnd);
      Pos.x = LOWORD(lParam);
      Pos.y = HIWORD(lParam);
      /* Select document */
      int Index = GetTabIndex(hWnd, LOWORD(lParam), HIWORD(lParam));
      SendMessage(GetParent(hWnd), WM_SELECTDOCUMENT, Index, 0);
      return 0;
    }
    case WM_LBUTTONUP:
    {
      if (Dragging)
      {
			  /* Drop item */
        Dragging = false;
        int Index = TabCtrl_GetCurSel(hWnd);
			  int Target = GetTabIndex(hWnd, LOWORD(lParam), HIWORD(lParam));
			  if (Index >= 0 && Target >= 0)
			  {
  			  /* Notify parent */
  			  if (DocumentMovedProc != NULL)
            (*DocumentMovedProc)(Index, Target);
        }
  			SetCursor(LoadCursor(NULL, IDC_ARROW));
      }
      ReleaseCapture();
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      if (Dragging)
      {
			  /* Change cursor */
			  int Index = GetTabIndex(hWnd, LOWORD(lParam), HIWORD(lParam));
			  if (Index >= 0 && Index != TabCtrl_GetCurSel(hWnd))
  			  SetCursor(LoadCursor(NULL, IDC_ARROW));
        else
	 		    SetCursor(LoadCursor(NULL, IDC_NO));
      }
      else if (wParam & MK_LBUTTON && (abs(LOWORD(lParam)-Pos.x) > 5 || abs(HIWORD(lParam)-Pos.y) > 5))
      {
        if (GetTabIndex(hWnd, LOWORD(lParam), HIWORD(lParam)) >= 0)
          Dragging = true;
      }
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)DefaultTabWndProc, hWnd, Msg, wParam, lParam);
}
