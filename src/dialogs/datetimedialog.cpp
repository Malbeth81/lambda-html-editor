#include <windows.h>
#include "../resource.h"
#include "../main.h"

char DateFormat[32];
char TimeFormat[32];
char DateTime[MAX_PATH];

// WINAPI FUNCTIONS ------------------------------------------------------------

void FillList(HWND hWnd, char* Text, int Count)
{
  for (Count; Count > 0; Count--)
  {
    SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)Text);
    Text += strlen(Text)+1;
  }
}

void FormatDateTime()
{
  SYSTEMTIME SysTime;
  GetSystemTime(&SysTime);
  char* Date = FormatDate(SysTime, 0, DateFormat);
  char* Time = FormatTime(SysTime, 0, TimeFormat);
  strcpy(DateTime, "");
  if (strlen(Date) > 0)
  {
    strcpy(DateTime, Date);
    if (strlen(Time) > 0)
    {
      strcat(DateTime, " ");
      strcat(DateTime, Time);
    }
  }
  else if (strlen(Time) > 0)
    strcpy(DateTime, Time);
  delete[] Date;
  delete[] Time;
}

BOOL CALLBACK DateTimeDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:        /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_DATEFORMATCMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            if (Index > 0)
              SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)DateFormat);
            else
              strcpy(DateFormat, "");
            FormatDateTime();
            SetWindowText(GetDlgItem(hDlg, ID_DATETIMEPREVIEWLBL), DateTime);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          else if (HIWORD(wParam) == CBN_EDITCHANGE)
          {
            char* Str = GetWindowText((HWND)lParam);
            strcpy(DateFormat, Str);
            delete[] Str;
            FormatDateTime();
            SetWindowText(GetDlgItem(hDlg, ID_DATETIMEPREVIEWLBL), DateTime);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_TIMEFORMATCMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            if (Index > 0)
              SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)TimeFormat);
            else
              strcpy(TimeFormat, "");
            FormatDateTime();
            SetWindowText(GetDlgItem(hDlg, ID_DATETIMEPREVIEWLBL), DateTime);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          else if (HIWORD(wParam) == CBN_EDITCHANGE)
          {
            char* Str = GetWindowText((HWND)lParam);
            strcpy(TimeFormat, Str);
            delete[] Str;
            FormatDateTime();
            SetWindowText(GetDlgItem(hDlg, ID_DATETIMEPREVIEWLBL), DateTime);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_OKBTN:
        {
          SendMessage(hDlg, WM_CLOSE, TRUE, 0);
          break;
        }
        case IDCANCEL:      // Generic message sent by IsDialogMessage for Esc key
        case ID_CANCELBTN:
        {
          SendMessage(hDlg, WM_CLOSE, FALSE, 0);
          break;
        }
      }
      return TRUE;
    }
    case WM_CLOSE:
    {
      EndDialog(hDlg, wParam);
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      FillList(GetDlgItem(hDlg, ID_DATEFORMATCMB), "None\0dd/MM/yyyy\0dd/MM/yy\0d/M/yy\0yyyy-MM-dd\0yy-MM-dd\0M/dd/yy\0MMMM d, yyyy\0d-MMM-yy", 9);
      FillList(GetDlgItem(hDlg, ID_TIMEFORMATCMB), "None\0HH:mm:ss\0H:mm:ss\0hh:mm:ss tt\0h:mm:ss tt", 5);
      if (SendDlgItemMessage(hDlg, ID_DATEFORMATCMB, CB_SELECTSTRING, (WPARAM)0, (LPARAM)DateFormat) == CB_ERR)
        SetDlgItemText(hDlg, ID_DATEFORMATCMB, DateFormat);
      if (SendDlgItemMessage(hDlg, ID_TIMEFORMATCMB, CB_SELECTSTRING, (WPARAM)0, (LPARAM)TimeFormat) == CB_ERR)
        SetDlgItemText(hDlg, ID_TIMEFORMATCMB, TimeFormat);
      SetWindowText(GetDlgItem(hDlg, ID_DATETIMEPREVIEWLBL), DateTime);
      return TRUE;
    }
  default:
    return FALSE;
  }
}

char* ShowDateTimeDialog()
{
  if (DialogBox(hInstance, MAKEINTRESOURCE(ID_DATETIMEDIALOG), hWindow, (DLGPROC)DateTimeDialogProc) == TRUE)
    return DateTime;
  else
    return "";
}
