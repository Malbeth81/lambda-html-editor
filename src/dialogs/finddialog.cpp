#include <windows.h>
#include "../resource.h"
#include "../main.h"
#include "../tools.h"

char* FindStr = "";
char* ReplaceStr = "";
bool MatchCase = false;
bool FromCaret = false;
bool WholeWords = false;
bool Down = true;

// PUBLIC FUNCTIONS ------------------------------------------------------------

void FindNext()
{
  Find(FindStr, FromCaret, Down, MatchCase, WholeWords);
}

// PRIVATE FUNCTIONS -----------------------------------------------------------

void GetOptions(HWND hDlg)
{
  delete[] FindStr;
  delete[] ReplaceStr;
  FindStr = GetWindowText(GetDlgItem(hDlg, ID_FINDCMB));
  ReplaceStr = GetWindowText(GetDlgItem(hDlg, ID_REPLACECMB));
  MatchCase = (IsDlgButtonChecked(hDlg, ID_MATCHCASECHK) == BST_CHECKED);
  FromCaret = (IsDlgButtonChecked(hDlg, ID_FROMCURSORCHK) == BST_CHECKED);
  WholeWords = (IsDlgButtonChecked(hDlg, ID_WHOLEWORDSCHK) == BST_CHECKED);
  Down = (IsDlgButtonChecked(hDlg, ID_NEXTRBN) == BST_CHECKED);
}

void SetOptions(HWND hDlg)
{
  SetWindowText(GetDlgItem(hDlg, ID_FINDCMB), FindStr);
  SetWindowText(GetDlgItem(hDlg, ID_REPLACECMB), ReplaceStr);
  CheckDlgButton(hDlg, ID_MATCHCASECHK, MatchCase ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, ID_FROMCURSORCHK, FromCaret ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, ID_WHOLEWORDSCHK, WholeWords ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, ID_NEXTRBN, Down ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton(hDlg, ID_PREVIOUSRBN, !Down ? BST_CHECKED : BST_UNCHECKED);
}

// WINAPI FUNCTIONS ------------------------------------------------------------

BOOL CALLBACK FindDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_COMMAND:        /* Process control messages */
    GetOptions(hDlg);
    switch (LOWORD(wParam))
    {
    case ID_OKBTN:
      Find(FindStr, FromCaret, Down, MatchCase, WholeWords);
      break;
    case ID_REPLACEFINDBTN:
      ReplaceSelection(ReplaceStr);
      Find(FindStr, FromCaret, Down, MatchCase, WholeWords);
      break;
    case ID_REPLACEBTN:
      ReplaceSelection(ReplaceStr);
      break;
    case ID_REPLACEALLBTN:
      ReplaceAll(FindStr, ReplaceStr, FromCaret, MatchCase, WholeWords);
      break;
    case IDCANCEL:      // Generic message sent by IsDialogMessage for Esc key
    case ID_CANCELBTN:
      SendMessage(hDlg, WM_CLOSE, 0, 0);
      break;
    }
    return TRUE;
  case WM_CLOSE:
    GetOptions(hDlg);
    EndDialog(hDlg, wParam);
    return TRUE;
  case WM_INITDIALOG:
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(ID_MAINICON), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION));
    SetOptions(hDlg);
    return TRUE;
  default:
    return FALSE;
  }
}

void ShowFindDialog()
{
  CreateDialog(hInstance, MAKEINTRESOURCE(ID_FINDDIALOG), hWindow, (DLGPROC)FindDialogProc);
}
