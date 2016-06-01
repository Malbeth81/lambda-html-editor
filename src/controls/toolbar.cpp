#include "toolbar.h"
#include "../resource.h"
#include "../menu.h"

LONG DefaultToolbarWndProc;

int ButtonIDs[NUM_BUTTONS] = {ID_FILE_NEW, ID_FILE_OPEN, -1, ID_FILE_SAVE, ID_FILE_SAVEALL,
                              ID_FILE_CLOSEDOC, -1, ID_FILE_PRINT, -1, ID_FILE_ADDPROJECTFILE,
                              ID_FILE_REMOVEFROMPROJECT, ID_FILE_ADDPROJECTFOLDER, -1,
                              ID_EDIT_UNDO, ID_EDIT_REDO, -1, ID_EDIT_COPY, ID_EDIT_CUT,
                              ID_EDIT_PASTE, -1, ID_EDIT_FIND, -1, ID_EDIT_INDENT,
                              ID_EDIT_UNINDENT, -1, ID_EDIT_PREFERENCES};

// PUBLIC FUNCTIONS ------------------------------------------------------------

HWND CreateToolbar(HWND Parent)
{
  /* Create the toolbar */
  HINSTANCE hInstance = GetModuleHandle(NULL);
  HWND Toolbar = CreateWindowEx(WS_EX_CLIENTEDGE, TOOLBARCLASSNAME, (LPSTR) NULL,
    WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NORESIZE | CCS_NODIVIDER
    , 0, 0, 0, 0, Parent, (HMENU) ID_TOOLBAR, hInstance, NULL);
  DefaultToolbarWndProc = SetWindowLong(Toolbar, GWL_WNDPROC, (LONG)&ToolbarWndProc);
  SendMessage(Toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

  /* Add the image list for the toolbar buttons */
  HIMAGELIST hList = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_COLOR16 | ILC_MASK, 0, 1);
  if (hList == NULL)
      return 0;
  HBITMAP Bitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(ID_TOOLBARICONS));
  ImageList_AddMasked(hList, Bitmap, RGB(255, 0, 255));
  DeleteObject(Bitmap);
  SendMessage(Toolbar, TB_SETIMAGELIST, 0, (LPARAM)hList);

  /* Add the toolbar buttons */
  TBBUTTON Buttons[NUM_BUTTONS];
  int ButtonIndex = 0;
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
  	Buttons[i].iBitmap = (ButtonIDs[i] == -1 ? -1 : ButtonIndex);
  	Buttons[i].idCommand = (ButtonIDs[i] == -1 ? -1 : ButtonIDs[i]);
  	Buttons[i].fsStyle = (ButtonIDs[i] == -1 ? TBSTYLE_SEP : TBSTYLE_BUTTON);
  	Buttons[i].fsState = TBSTATE_ENABLED;
  	Buttons[i].dwData = 0;
  	Buttons[i].iString = 0;
  	if (ButtonIDs[i] >= 0)
      ButtonIndex ++;
  }
  SendMessage(Toolbar, TB_ADDBUTTONS, (WPARAM)NUM_BUTTONS, (LPARAM)&Buttons);
  return Toolbar;
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT CALLBACK ToolbarWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_NOTIFY:
    {
      switch (((LPNMHDR) lParam)->code)
      {
        case TTN_GETDISPINFOA:
        case TTN_GETDISPINFOW:
        {
          LPNMTTDISPINFO Info = (LPNMTTDISPINFO) lParam;
          Info->hinst = GetModuleHandle(NULL);
          Info->lpszText = MAKEINTRESOURCE(Info->hdr.idFrom);
          break;
        }
      }
      return 0;
    }
    case WM_NCPAINT:
    {
      RECT R;
      GetWindowRect(hWnd, &R);
      R.right -= R.left;
      R.bottom -= R.top;
      R.left = 0;
      R.top = 0;
      HDC DC = GetWindowDC(hWnd);
      DrawEdge(DC, &R, EDGE_ETCHED, BF_RECT);
      ReleaseDC(hWnd, DC);
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)DefaultToolbarWndProc, hWnd, Msg, wParam, lParam);
}
