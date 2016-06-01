#include <windows.h>
#include "propertiesdialog.h"
#include "../main.h"
#include "../resource.h"
#include "../tools.h"

char* File;
char* Folder;
char* CreatedTime;
char* ModifiedTime;
char* Size;

void DrawHorzLine(HWND hWnd, LPDRAWITEMSTRUCT DrawStruct)
{
}

LONG OldEditWndProc;
LRESULT CALLBACK EditWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_NCPAINT:  // Disable the painting of the border
      return 0;
  }
  return CallWindowProc((WNDPROC)OldEditWndProc, hWnd, Msg, wParam, lParam);
}

LONG OldStaticWndProc;
LRESULT CALLBACK HorzLineWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_PAINT:
    {
      /* Gets window size */
    	RECT R;
    	GetClientRect(hWnd, &R);
    	/* Paints the frame */
      PAINTSTRUCT PS;
    	HDC DC = BeginPaint(hWnd, &PS);
      DrawEdge(DC, &R, BDR_SUNKENOUTER, BF_RECT);
      EndPaint(hWnd, &PS);
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)OldStaticWndProc, hWnd, Msg, wParam, lParam);
}

BOOL CALLBACK PropertiesDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:        /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_SYSPROPERTIESBTN: {
          char* FileName = new char[MAX_PATH];
          strcpy(FileName, Folder);
          strcat(FileName, File);
          OpenFileProperties(FileName);
          delete[] FileName;
        } break;
        case IDCANCEL:      // Generic message sent by IsDialogMessage for Esc key
        case ID_CANCELBTN:
          SendMessage(hDlg, WM_CLOSE, 0, 0);
          break;
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      EndDialog(hDlg, wParam);
      return TRUE;
    }
  	case WM_DRAWITEM:
    {
      DrawHorzLine(GetDlgItem(hDlg, (UINT)wParam), (LPDRAWITEMSTRUCT)lParam);
  		return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Override the window procs */
      OldEditWndProc = SetWindowLong(GetDlgItem(hDlg, ID_NAMEEDT), GWL_WNDPROC, (LONG)&EditWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_LOCATIONEDT), GWL_WNDPROC, (LONG)&EditWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_TYPEEDT), GWL_WNDPROC, (LONG)&EditWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_DATECREATEDEDT), GWL_WNDPROC, (LONG)&EditWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_DATEMODIFIEDEDT), GWL_WNDPROC, (LONG)&EditWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_SIZEEDT), GWL_WNDPROC, (LONG)&EditWndProc);

      OldStaticWndProc = SetWindowLong(GetDlgItem(hDlg, ID_HORZLINE1), GWL_WNDPROC, (LONG)&HorzLineWndProc);
      SetWindowLong(GetDlgItem(hDlg, ID_HORZLINE2), GWL_WNDPROC, (LONG)&HorzLineWndProc);

      /* Set the control values */
      SetWindowText(GetDlgItem(hDlg, ID_NAMEEDT), File);
      SetWindowText(GetDlgItem(hDlg, ID_LOCATIONEDT), Folder);
      SetWindowText(GetDlgItem(hDlg, ID_TYPEEDT), "");
      SetWindowText(GetDlgItem(hDlg, ID_DATECREATEDEDT), CreatedTime);
      SetWindowText(GetDlgItem(hDlg, ID_DATEMODIFIEDEDT), ModifiedTime);
      SetWindowText(GetDlgItem(hDlg, ID_SIZEEDT), Size);

      PostMessage(GetDlgItem(hDlg, ID_NAMEEDT), EM_SETSEL, 0, 0);
      PostMessage(GetDlgItem(hDlg, ID_LOCATIONEDT), EM_SETSEL, 0, 0);
      PostMessage(GetDlgItem(hDlg, ID_TYPEEDT), EM_SETSEL, 0, 0);
      PostMessage(GetDlgItem(hDlg, ID_DATECREATEDEDT), EM_SETSEL, 0, 0);
      PostMessage(GetDlgItem(hDlg, ID_DATEMODIFIEDEDT), EM_SETSEL, 0, 0);
      PostMessage(GetDlgItem(hDlg, ID_SIZEEDT), EM_SETSEL, 0, 0);

      SetFocus(GetDlgItem(hDlg, ID_CANCELBTN));
      return TRUE;
    }
    default:
      return FALSE;
  }
}

void ShowPropertiesDialog(string FileName)
{
  /* Format the display of the file name and path */
  File = new char[MAX_PATH];
  Folder = new char[MAX_PATH];
  strcpy(File, FileName.substr(FileName.rfind('\\')+1).c_str());
  strcpy(Folder, FileName.substr(0, FileName.rfind('\\')+1).c_str());
  /* Format the display of the creation date and time */
  FILETIME FileTime;
  if (GetFileCreationDate(FileName.c_str(), &FileTime))
  {
    SYSTEMTIME Time;
    FileTimeToSystemTime(&FileTime, &Time);
    CreatedTime = FormatDate(Time, LOCALE_NOUSEROVERRIDE, NULL);
    strcat(CreatedTime, ", ");
    char* Str = FormatTime(Time, LOCALE_NOUSEROVERRIDE, NULL);
    strcat(CreatedTime, Str);
    delete[] Str;
  }
  /* Format the display of the modification date and time */
  FileTime;
  if (GetFileModificationDate(FileName.c_str(), &FileTime))
  {
    SYSTEMTIME Time;
    FileTimeToSystemTime(&FileTime, &Time);
    ModifiedTime = FormatDate(Time, LOCALE_NOUSEROVERRIDE, NULL);
    strcat(ModifiedTime, ", ");
    char* Str = FormatTime(Time, LOCALE_NOUSEROVERRIDE, NULL);
    strcat(ModifiedTime, Str);
    delete[] Str;
  }
  /* Format the display of the file size */
  Size = FormatSize(GetFileSize(FileName.c_str()));
  /* Show the dialog */
  DialogBox(hInstance, MAKEINTRESOURCE(ID_PROPERTIESDIALOG), hWindow, (DLGPROC)PropertiesDialogProc);
  delete[] File;
  delete[] Folder;
  delete[] ModifiedTime;
  delete[] Size;
}
