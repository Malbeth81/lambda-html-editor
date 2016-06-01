#include <windows.h>
#include "../resource.h"
#include "../main.h"
#include "../controls/scrolledit.h"

// WINAPI FUNCTIONS ------------------------------------------------------------

BOOL CALLBACK GotoDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_COMMAND:        /* Process control messages */
    switch (LOWORD(wParam))
    {
    case ID_OKBTN: {
      BOOL Success;
      int Line = GetDlgItemInt(hDlg, ID_LINEEDT, &Success, FALSE)-1;
      if (Success == TRUE)
      {
        int Col = GetDlgItemInt(hDlg, ID_COLLUMNEDT, &Success, FALSE)-1;
        if (Success == TRUE)
        {
          Editor->GetDocument()->SetCaretPos(Col, Line);
          SendMessage(hDlg, WM_CLOSE, 1, 0);
        }
      }
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
  case WM_INITDIALOG: {
    /* Set up controls */
    MakeEditScrollable(GetDlgItem(hDlg, ID_LINEEDT));
    MakeEditScrollable(GetDlgItem(hDlg, ID_COLLUMNEDT));
    /* Display data */
    char* Str = IntToStr(Editor->GetDocument()->GetCaretPos().y+1);
    SetWindowText(GetDlgItem(hDlg, ID_LINEEDT), Str);
    delete[] Str;
    Str = IntToStr(Editor->GetDocument()->GetCaretPos().x+1);
    SetWindowText(GetDlgItem(hDlg, ID_COLLUMNEDT), Str);
    delete[] Str;
  } return TRUE;
  default:
    return FALSE;
  }
}

void ShowGotoDialog()
{
  DialogBox(hInstance, MAKEINTRESOURCE(ID_GOTODIALOG), hWindow, (DLGPROC)GotoDialogProc);
}
