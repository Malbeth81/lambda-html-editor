#include <windows.h>
#include "inputdialog.h"
#include "../resource.h"
#include "../main.h"

const char* DlgTitle;
const char* DlgPrompt;
char* DlgValue;

BOOL CALLBACK InputDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_COMMAND:        /* Process control messages */
    switch (LOWORD(wParam))
    {
    case ID_OKBTN: {
      GetWindowText(GetDlgItem(hDlg, ID_INPUTEDT), DlgValue, MAX_PATH);
      SendMessage(hDlg, WM_CLOSE, 1, 0);
    } break;
    case IDCANCEL:      // Generic message sent by IsDialogMessage for Esc key
    case ID_CANCELBTN:
      SendMessage(hDlg, WM_CLOSE, 0, 0);
      break;
    }
    return TRUE;
  case WM_CLOSE:
    EndDialog(hDlg, wParam);
    return TRUE;
  case WM_INITDIALOG:
    SetWindowText(hDlg, DlgTitle);
    SetWindowText(GetDlgItem(hDlg, ID_INPUTLBL), DlgPrompt);
    SetWindowText(GetDlgItem(hDlg, ID_INPUTEDT), DlgValue);
    return TRUE;
  default:
    return FALSE;
  }
}

string InputDialog(const char* Title, const char* Prompt, string Value)
{
  string Result = "";
  DlgTitle = Title;
  DlgPrompt = Prompt;
  DlgValue = new char[MAX_PATH];
  strcpy(DlgValue, Value.c_str());
  if (DialogBox(hInstance, MAKEINTRESOURCE(ID_INPUTDIALOG), hWindow, (DLGPROC)InputDialogProc) > 0)
    Result.assign(DlgValue);
  delete[] DlgValue;
  return Result;
}
