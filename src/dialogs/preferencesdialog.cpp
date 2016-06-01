#include <windows.h>
#include "../resource.h"
#include "../main.h"
#include "../controls/splitter.h"
#include "../controls/scrolledit.h"

TFont EditorFont;
TFont GutterFont;

// PRIVATE FUNCTIONS -----------------------------------------------------------

int CALLBACK EnumFontProc(const LOGFONTA *LogFont, const TEXTMETRICA *Metric, DWORD FontType, LPARAM lParam)
{
  if (LogFont->lfPitchAndFamily & FIXED_PITCH && LogFont->lfFaceName[0] != '@')
    if (SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, 0, (LPARAM)LogFont->lfFaceName) == CB_ERR)
      SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)LogFont->lfFaceName);
  return 1;
}

void ListFonts(HWND hWnd)
{
  LOGFONT LFont;
  LFont.lfCharSet = DEFAULT_CHARSET;
  LFont.lfFaceName[0] = '\0';
  HDC DC = GetDC(0);
  EnumFontFamiliesEx(DC, &LFont, (FONTENUMPROC)EnumFontProc, (LPARAM)hWnd, 0);
  ReleaseDC(0, DC);
}

int CALLBACK EnumFontSizeProc(const LOGFONTA *LogFont, const TEXTMETRICA *Metric, DWORD FontType, LPARAM lParam)
{
  static const unsigned short int TrueTypeSizes[13] = {8,9,10,11,12,14,16,18,20,22,24,26,28};
  if (FontType & TRUETYPE_FONTTYPE)
  {
    for (int i = 0; i < 13; i++)
    {
    	char* Str = IntToStr(TrueTypeSizes[i]);
      SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)Str);
      delete[] Str;
    }
    return 0;
  }
  else
  {
    HDC DC = GetDC(0);
    int Size = MulDiv(Metric->tmHeight-Metric->tmInternalLeading, 72, GetDeviceCaps(DC, LOGPIXELSY));
    char* Str = IntToStr(Size);
    if (SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, 0, (LPARAM)Str) == CB_ERR)
      SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)Str);
    ReleaseDC(0, DC);
    return 1;
  }
}

void ListFontSizes(HWND hWnd, const char* FaceName)
{
  int Size = atoi(GetWindowText(hWnd));
  SendMessage(hWnd, CB_RESETCONTENT, 0, 0);
  LOGFONT LFont;
  LFont.lfCharSet = DEFAULT_CHARSET;
  strncpy(LFont.lfFaceName, FaceName, LF_FACESIZE);
  HDC DC = GetDC(0);
  EnumFontFamiliesEx(DC, &LFont, (FONTENUMPROC)EnumFontSizeProc, (LPARAM)hWnd, 0);
  ReleaseDC(0, DC);
  char* Str = IntToStr(Size);
  if (SendMessage(hWnd, CB_SELECTSTRING, (WPARAM)0, (LPARAM)Str) == CB_ERR)
    SetWindowText(hWnd, Str);
  delete[] Str;
}

void PreviewFont(HWND hWnd, PFont Font)
{
  HDC DC = GetWindowDC(hWnd);
  SendMessage(hWnd, WM_SETFONT, (WPARAM)EasyCreateFont(DC, Font), TRUE);
  ReleaseDC(hWnd, DC);
}

void AssociateTo(char* Ext, bool SetOpen)
{
  try
  {
    HKEY Key;
    DWORD Size = MAX_PATH;
    /* Open or create the key for this extension */
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, Ext, 0, KEY_ALL_ACCESS, &Key) != ERROR_SUCCESS)
      RegCreateKey(HKEY_CLASSES_ROOT, Ext, &Key);
    else
    {
      /* Get or set the key associated to this extension */
      char* Value = new char[Size];
      if (RegQueryValueEx(Key, NULL, NULL, NULL, (BYTE*)Value, &Size) != ERROR_SUCCESS)
      {
        strcpy(Value, "LambdaDocument");
        RegSetValueEx(Key, NULL, 0, REG_SZ, (BYTE*)Value, strlen(Value)+1);
        RegCloseKey(Key);
      }
      /* Open or create the key associated to this extension */
      if (RegOpenKeyEx(HKEY_CLASSES_ROOT, Value, 0, KEY_ALL_ACCESS, &Key) != ERROR_SUCCESS)
      {
        RegCreateKey(HKEY_CLASSES_ROOT, Value, &Key);
        RegCloseKey(Key);
      }
      /* Set the icon associated to this extension */
      if (SetOpen)
      {
        char* Str = new char[MAX_PATH];
        strcpy(Str, Value);
        strcat(Str, "\\DefaultIcon");
        if (RegOpenKey(HKEY_CLASSES_ROOT, Str, &Key) == ERROR_SUCCESS)
        {
          char* FileName = new char[MAX_PATH];
          GetModuleFileName(hInstance, FileName, MAX_PATH);
          strcat(FileName, ",1");
          RegSetValueEx(Key, "", 0, REG_SZ, (BYTE*)FileName, strlen(FileName)+1);
          RegCloseKey(Key);
          delete[] FileName;
        }
        delete[] Str;
      }
      /* Set the command */
      if (SetOpen)
        strcat(Value, "\\shell\\open\\command");
      else
        strcat(Value, "\\shell\\edit\\command");
      if (RegOpenKey(HKEY_CLASSES_ROOT, Value, &Key) == ERROR_SUCCESS)
      {
        char* FileName = new char[MAX_PATH];
        GetModuleFileName(hInstance, FileName, MAX_PATH);
        strcat(FileName, " \"%1\"");
        RegSetValueEx(Key, NULL, 0, REG_SZ, (BYTE*)FileName, strlen(FileName)+1);
        RegCloseKey(Key);
        delete[] FileName;
      }
      delete[] Value;
    }
  }
  catch (exception e)
  {
  }
}

bool IsAssociateTo(char* Ext, bool HasOpen)
{
  bool Result = false;
  try
  {
    HKEY Key;
    DWORD Size = MAX_PATH;
    /* Open or create the key for this extension */
    if (RegOpenKey(HKEY_CLASSES_ROOT, Ext, &Key) == ERROR_SUCCESS)
    {
      /* Get the key associated to this extension */
      char* Value = new char[Size];
      RegQueryValueEx(Key, NULL, NULL, NULL, (BYTE*)Value, &Size);
      RegCloseKey(Key);
      /* Validate associated command */
      if (HasOpen)
        strcat(Value, "\\shell\\open\\command");
      else
        strcat(Value, "\\shell\\edit\\command");
      if (RegOpenKey(HKEY_CLASSES_ROOT, Value, &Key) == ERROR_SUCCESS)
      {
        /* Get the command */
        DWORD Size2 = MAX_PATH;
        char* Value2 = new char[Size2];
        RegQueryValueEx(Key, NULL, NULL, NULL, (BYTE*)Value2, &Size2);
        RegCloseKey(Key);
        char* FileName = new char[MAX_PATH];
        GetModuleFileName(hInstance, FileName, MAX_PATH);
        strcat(FileName, " \"%1\"");
        /* Compare the command with the current file name */
        if (strcmp(Value2, FileName) == 0)
          Result = true;
        delete[] FileName;
        delete[] Value2;
      }
      delete[] Value;
    }
  }
  catch (exception e)
  {
  }
  return Result;
}

void RemoveAssociation(char* Ext, bool Open)
{
  try
  {
    HKEY Key;
    DWORD Size = MAX_PATH;
    /* Open or create the key for this extension */
    if (RegOpenKey(HKEY_CLASSES_ROOT, Ext, &Key) == ERROR_SUCCESS)
    {
      /* Get the key associated to this extension */
      char* Value = new char[Size];
      RegQueryValueEx(Key, NULL, NULL, NULL, (BYTE*)Value, &Size);
      RegCloseKey(Key);
      /* Delete the command */
      if (Open)
        strcat(Value, "\\shell\\open\\command");
      else
        strcat(Value, "\\shell\\edit\\command");
      if (RegOpenKey(HKEY_CLASSES_ROOT, Value, &Key) == ERROR_SUCCESS)
      {
        RegDeleteValue(Key, NULL);
        RegCloseKey(Key);
      }
      delete[] Value;
    }
  }
  catch (exception e)
  {
  }
}

// WINAPI FUNCTIONS ------------------------------------------------------------

BOOL CALLBACK PreferencesDialog1Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:      /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_AUTOUPDATECHK:
        case ID_OVERWRITEEMPTYDOCSCHK:
        case ID_HIDEEXPLORERCHK:
        case ID_OPENEXPFILEONSELCHK:
        case ID_RENAMEEXPONSELCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_AUTOHIDEEXPLORERCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            bool Checked = IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED;
            EnableWindow(GetDlgItem(hDlg, ID_AUTOHIDEDELAYEDT), Checked);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_DEFAULTBROWSERCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            bool Checked = IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED;
            EnableWindow(GetDlgItem(hDlg, ID_INTERNETBROWSEREDT), !Checked);
            EnableWindow(GetDlgItem(hDlg, ID_INTERNETBROWSERBTN), !Checked);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_AUTOHIDEDELAYEDT:
        case ID_INTERNETBROWSEREDT:
        {
          if (HIWORD(wParam) == EN_CHANGE)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
      }
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Set up controls */
      MakeEditScrollable(GetDlgItem(hDlg, ID_AUTOHIDEDELAYEDT));
      /* Display Preferences */
      CheckDlgButton(hDlg, ID_MULTIPLEINSTANCESCHK, AllowMultipleInstance ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_AUTOUPDATECHK, AutoUpdate ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_OVERWRITEEMPTYDOCSCHK, Editor->OverwriteEmptyDocs ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_OPENUNSUPPORTEDDOCSCHK, OpenUnsupportedFiles ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_HIDEEXPLORERCHK, (Explorer->GetHide() & HideSplitter) ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_OPENEXPFILEONSELCHK, Explorer->SingleClickOpen ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_RENAMEEXPONSELCHK, Explorer->RenameOnClick ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_AUTOHIDEEXPLORERCHK, Explorer->GetAutoHide() ? BST_CHECKED : BST_UNCHECKED);
      SetDlgItemInt(hDlg, ID_AUTOHIDEDELAYEDT, Explorer->AutoHideDelay/1000, TRUE);
      CheckDlgButton(hDlg, ID_DEFAULTBROWSERCHK, DefaultInternetBrowser ? BST_CHECKED : BST_UNCHECKED);
      SetWindowText(GetDlgItem(hDlg, ID_INTERNETBROWSEREDT), InternetBrowser.c_str());
      /* Apply control modifiations */
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_DEFAULTBROWSERCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_DEFAULTBROWSERCHK));
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_AUTOHIDEEXPLORERCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_AUTOHIDEEXPLORERCHK));
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_APPLY:
        {
          /* Save Preferences */
          AllowMultipleInstance = IsDlgButtonChecked(hDlg, ID_MULTIPLEINSTANCESCHK) == BST_CHECKED;
          AutoUpdate = IsDlgButtonChecked(hDlg, ID_AUTOUPDATECHK) == BST_CHECKED;
          Editor->OverwriteEmptyDocs = IsDlgButtonChecked(hDlg, ID_OVERWRITEEMPTYDOCSCHK) == BST_CHECKED;
          OpenUnsupportedFiles = IsDlgButtonChecked(hDlg, ID_OPENUNSUPPORTEDDOCSCHK) == BST_CHECKED;
          HideSplitter = IsDlgButtonChecked(hDlg, ID_HIDEEXPLORERCHK) == BST_CHECKED;
          Explorer->SetHide(HideSplitter);
          Explorer->SingleClickOpen = IsDlgButtonChecked(hDlg, ID_OPENEXPFILEONSELCHK) == BST_CHECKED;
          Explorer->RenameOnClick = IsDlgButtonChecked(hDlg, ID_RENAMEEXPONSELCHK) == BST_CHECKED;
          Explorer->SetAutoHide(IsDlgButtonChecked(hDlg, ID_AUTOHIDEEXPLORERCHK) == BST_CHECKED);
          Explorer->AutoHideDelay = GetDlgItemInt(hDlg, ID_AUTOHIDEDELAYEDT, NULL, FALSE)*1000;
          DefaultInternetBrowser = IsDlgButtonChecked(hDlg, ID_DEFAULTBROWSERCHK) == BST_CHECKED;
          InternetBrowser = GetWindowText(GetDlgItem(hDlg, ID_INTERNETBROWSEREDT));
          return TRUE;
        }
      }
      return FALSE;
    }
    default:
      return FALSE;
  }
}

BOOL CALLBACK PreferencesDialog2Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:      /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_OVERWRITESELECTIONCHK:
        case ID_OVERWRITETEXTCHK:
        case ID_AUTOINDENTCHK:
        case ID_GROUPUNDOCHK:
        case ID_DBLCLKSELLINECHK:
        case ID_SPACESEPARATEDWORDSCHK:
        case ID_TABINDENTSLINESCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_DEFAULTSCROLLCOUNTCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            bool Checked = IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED;
            EnableWindow(GetDlgItem(hDlg, ID_SCROLLLINECOUNTEDT), !Checked);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_INDENTSIZEEDT:
        case ID_SCROLLLINECOUNTEDT:
        {
          if (HIWORD(wParam) == EN_CHANGE)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
      }
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Set up controls */
      MakeEditScrollable(GetDlgItem(hDlg, ID_INDENTSIZEEDT));
      MakeEditScrollable(GetDlgItem(hDlg, ID_SCROLLLINECOUNTEDT));
      /* Display Preferences */
      CheckDlgButton(hDlg, ID_OVERWRITESELECTIONCHK, Editor->OverwriteSelection ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_OVERWRITETEXTCHK, Editor->OverwriteMode ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_AUTOINDENTCHK, Editor->AutoIndent ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_GROUPUNDOCHK, Editor->GroupUndo ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_DBLCLKSELLINECHK, Editor->DoubleClickLine ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_SPACESEPARATEDWORDSCHK, Editor->SpaceSeparatedWords ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_TABINDENTSLINESCHK, Editor->TabIndentsLines ? BST_CHECKED : BST_UNCHECKED);
      SetDlgItemInt(hDlg, ID_INDENTSIZEEDT, Editor->TabSize, TRUE);
      CheckDlgButton(hDlg, ID_DEFAULTSCROLLCOUNTCHK, Editor->DefaultScrollLineCount ? BST_CHECKED : BST_UNCHECKED);
      char* Str = IntToStr(Editor->ScrollLineCount);
      SetWindowText(GetDlgItem(hDlg, ID_SCROLLLINECOUNTEDT), Str);
      delete[] Str;
      /* Apply control modifiations */
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_DEFAULTSCROLLCOUNTCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_DEFAULTSCROLLCOUNTCHK));
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_APPLY:
        {
          /* Save Preferences */
          Editor->OverwriteSelection = IsDlgButtonChecked(hDlg, ID_OVERWRITESELECTIONCHK) == BST_CHECKED;
          Editor->OverwriteMode = IsDlgButtonChecked(hDlg, ID_OVERWRITETEXTCHK) == BST_CHECKED;
          Editor->AutoIndent = IsDlgButtonChecked(hDlg, ID_AUTOINDENTCHK) == BST_CHECKED;
          Editor->GroupUndo = IsDlgButtonChecked(hDlg, ID_GROUPUNDOCHK) == BST_CHECKED;
          Editor->DoubleClickLine = IsDlgButtonChecked(hDlg, ID_DBLCLKSELLINECHK) == BST_CHECKED;
          Editor->SpaceSeparatedWords = IsDlgButtonChecked(hDlg, ID_SPACESEPARATEDWORDSCHK) == BST_CHECKED;
          Editor->TabIndentsLines = IsDlgButtonChecked(hDlg, ID_TABINDENTSLINESCHK) == BST_CHECKED;
          Editor->TabSize = GetDlgItemInt(hDlg, ID_INDENTSIZEEDT, NULL, FALSE);
          Editor->DefaultScrollLineCount = IsDlgButtonChecked(hDlg, ID_DEFAULTSCROLLCOUNTCHK) == BST_CHECKED;
          Editor->ScrollLineCount = GetDlgItemInt(hDlg, ID_SCROLLLINECOUNTEDT, NULL, FALSE);
          return TRUE;
        }
      }
      return FALSE;
    }
    default:
      return FALSE;
  }
}

BOOL CALLBACK PreferencesDialog3Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:      /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_SHOWGUTTERCHK:
        case ID_SHOWMARGINCHK:
        case ID_SHOWLINENUMBERSCHK:
        case ID_AUTOSIZEGUTTERCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_GUTTERSIZEEDT:
        case ID_MARGINSIZEEDT:
        {
          if (HIWORD(wParam) == EN_CHANGE)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_EDITORFONTCMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            if (SendMessage((HWND)lParam, CB_GETLBTEXTLEN, Index, 0) <= 32)
            {
              SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)EditorFont.Name);
              ListFontSizes(GetDlgItem(hDlg, ID_EDITORFONTSIZECMB), EditorFont.Name);
              PreviewFont(GetDlgItem(hDlg, ID_EDITORFONTPREVIEWLBL), &EditorFont);
              SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            }
          }
          break;
        }
        case ID_EDITORFONTSIZECMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            char* Size = new char[33];
            SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)Size);
            EditorFont.Size = atoi(Size);
            delete[] Size;
            PreviewFont(GetDlgItem(hDlg, ID_EDITORFONTPREVIEWLBL), &EditorFont);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          else if (HIWORD(wParam) == CBN_EDITCHANGE)
          {
            char* Size = GetWindowText((HWND)lParam);
            EditorFont.Size = atoi(Size);
            delete[] Size;
            PreviewFont(GetDlgItem(hDlg, ID_EDITORFONTPREVIEWLBL), &EditorFont);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_GUTTERFONTCMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            if (SendMessage((HWND)lParam, CB_GETLBTEXTLEN, Index, 0) <= 32)
            {
              SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)GutterFont.Name);
              ListFontSizes(GetDlgItem(hDlg, ID_GUTTERFONTSIZECMB), GutterFont.Name);
              PreviewFont(GetDlgItem(hDlg, ID_GUTTERFONTPREVIEWLBL), &GutterFont);
              SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            }
          }
          break;
        }
        case ID_GUTTERFONTSIZECMB:
        {
          if (HIWORD(wParam) == CBN_SELCHANGE)
          {
            int Index = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            char* Size = new char[33];
            SendMessage((HWND)lParam, CB_GETLBTEXT, Index, (LPARAM)Size);
            GutterFont.Size = atoi(Size);
            delete[] Size;
            PreviewFont(GetDlgItem(hDlg, ID_GUTTERFONTPREVIEWLBL), &GutterFont);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          else if (HIWORD(wParam) == CBN_EDITCHANGE)
          {
            char* Size = GetWindowText((HWND)lParam);
            GutterFont.Size = atoi(Size);
            delete[] Size;
            PreviewFont(GetDlgItem(hDlg, ID_GUTTERFONTPREVIEWLBL), &GutterFont);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
      }
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      EditorFont = Editor->GetFont();
      GutterFont = Editor->GetGutterFont();
      /* Set up controls */
      MakeEditScrollable(GetDlgItem(hDlg, ID_GUTTERSIZEEDT));
      MakeEditScrollable(GetDlgItem(hDlg, ID_MARGINSIZEEDT));
      ListFonts(GetDlgItem(hDlg, ID_EDITORFONTCMB));
      ListFontSizes(GetDlgItem(hDlg, ID_EDITORFONTSIZECMB), EditorFont.Name);
      PreviewFont(GetDlgItem(hDlg, ID_EDITORFONTPREVIEWLBL), &EditorFont);
      ListFonts(GetDlgItem(hDlg, ID_GUTTERFONTCMB));
      ListFontSizes(GetDlgItem(hDlg, ID_GUTTERFONTSIZECMB), GutterFont.Name);
      PreviewFont(GetDlgItem(hDlg, ID_GUTTERFONTPREVIEWLBL), &GutterFont);
      /* Display Preferences */
      CheckDlgButton(hDlg, ID_SHOWGUTTERCHK, Editor->GetGutterVisible() ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_SHOWMARGINCHK, Editor->GetMarginVisible() ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_SHOWLINENUMBERSCHK, Editor->GetLineNumbersVisible() ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hDlg, ID_AUTOSIZEGUTTERCHK, Editor->GetAutoSizeGutter() ? BST_CHECKED : BST_UNCHECKED);
      SetDlgItemInt(hDlg, ID_GUTTERSIZEEDT, Editor->GetGutterSize(), FALSE);
      SetDlgItemInt(hDlg, ID_MARGINSIZEEDT, Editor->GetMarginSize(), FALSE);
      SendDlgItemMessage(hDlg, ID_EDITORFONTCMB, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)EditorFont.Name);
      char* Str = IntToStr(EditorFont.Size);
      if (SendDlgItemMessage(hDlg, ID_EDITORFONTSIZECMB, CB_SELECTSTRING, (WPARAM)0, (LPARAM)Str) == CB_ERR)
        SetDlgItemText(hDlg, ID_EDITORFONTSIZECMB, Str);
      delete[] Str;
      SendDlgItemMessage(hDlg, ID_GUTTERFONTCMB, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)GutterFont.Name);
      Str = IntToStr(GutterFont.Size);
      if (SendDlgItemMessage(hDlg, ID_GUTTERFONTSIZECMB, CB_SELECTSTRING, (WPARAM)0, (LPARAM)Str) == CB_ERR)
        SetDlgItemText(hDlg, ID_GUTTERFONTSIZECMB, Str);
      delete[] Str;
      /* Apply control modifiations */
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_SHOWMARGINCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_SHOWMARGINCHK));
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_SHOWGUTTERCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_SHOWGUTTERCHK));
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_SHOWLINENUMBERSCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_SHOWLINENUMBERSCHK));
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ID_AUTOSIZEGUTTERCHK, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ID_AUTOSIZEGUTTERCHK));
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_APPLY:
        {
          /* Save Preferences */
          Editor->SetGutterVisible(IsDlgButtonChecked(hDlg, ID_SHOWGUTTERCHK) == BST_CHECKED);
          Editor->SetMarginVisible(IsDlgButtonChecked(hDlg, ID_SHOWMARGINCHK) == BST_CHECKED);
          Editor->SetLineNumbersVisible(IsDlgButtonChecked(hDlg, ID_SHOWLINENUMBERSCHK) == BST_CHECKED);
          Editor->SetAutoSizeGutter(IsDlgButtonChecked(hDlg, ID_AUTOSIZEGUTTERCHK) == BST_CHECKED);
          Editor->SetGutterSize(GetDlgItemInt(hDlg, ID_GUTTERSIZEEDT, NULL, FALSE));
          Editor->SetMarginSize(GetDlgItemInt(hDlg, ID_MARGINSIZEEDT, NULL, FALSE));
          Editor->SetFont(EditorFont);
          Editor->SetGutterFont(GutterFont);
          return TRUE;
        }
      }
      return FALSE;
    }
    default:
      return FALSE;
  }
}

BOOL CALLBACK PreferencesDialog4Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:      /* Process control messages */
    {
      //switch (LOWORD(wParam))
      //{
      //}
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Set up controls */
      /* Display Preferences */
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_APPLY:
        {
          /* Save */
          return TRUE;
        }
      }
      return FALSE;
    }
    default:
      return FALSE;
  }
}

BOOL CALLBACK PreferencesDialog5Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:      /* Process control messages */
    {
      switch (LOWORD(wParam))
      {
        case ID_TEXTFILESCHK:
        case ID_HTMLFILESCHK:
        case ID_STYLESHEETFILESCHK:
        case ID_JAVASCRIPTFILESCHK:
        case ID_PHPFILESCHK:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_ALLFILESBTN:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            CheckDlgButton(hDlg, ID_TEXTFILESCHK, BST_CHECKED);
            CheckDlgButton(hDlg, ID_HTMLFILESCHK, BST_CHECKED);
            CheckDlgButton(hDlg, ID_STYLESHEETFILESCHK, BST_CHECKED);
            CheckDlgButton(hDlg, ID_JAVASCRIPTFILESCHK, BST_CHECKED);
            CheckDlgButton(hDlg, ID_PHPFILESCHK, BST_CHECKED);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
        case ID_NOFILESBTN:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            CheckDlgButton(hDlg, ID_TEXTFILESCHK, BST_UNCHECKED);
            CheckDlgButton(hDlg, ID_HTMLFILESCHK, BST_UNCHECKED);
            CheckDlgButton(hDlg, ID_STYLESHEETFILESCHK, BST_UNCHECKED);
            CheckDlgButton(hDlg, ID_JAVASCRIPTFILESCHK, BST_UNCHECKED);
            CheckDlgButton(hDlg, ID_PHPFILESCHK, BST_UNCHECKED);
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
          }
          break;
        }
      }
      return TRUE;
    }
    case WM_INITDIALOG:
    {
      /* Set up controls */
      /* Display Preferences */
      int Count = IsAssociateTo(".txt", true) + IsAssociateTo(".ini", true) +
                  IsAssociateTo(".inf", true) + IsAssociateTo(".log", true);
      if (Count == 4)
        CheckDlgButton(hDlg, ID_TEXTFILESCHK, BST_CHECKED);
      else if (Count > 0)
        CheckDlgButton(hDlg, ID_TEXTFILESCHK, BST_INDETERMINATE);
      else
        CheckDlgButton(hDlg, ID_TEXTFILESCHK, BST_UNCHECKED);

      Count = IsAssociateTo(".html", true) + IsAssociateTo(".htm", true) +
              IsAssociateTo(".htt", true) + IsAssociateTo(".shtml", true) +
              IsAssociateTo(".xht", true) + IsAssociateTo(".xhtml", true) +
              IsAssociateTo(".xml", true);
      if (Count == 7)
        CheckDlgButton(hDlg, ID_HTMLFILESCHK, BST_CHECKED);
      else if (Count > 0)
        CheckDlgButton(hDlg, ID_HTMLFILESCHK, BST_INDETERMINATE);
      else
        CheckDlgButton(hDlg, ID_HTMLFILESCHK, BST_UNCHECKED);

      Count = IsAssociateTo(".css", true) + IsAssociateTo(".xsl", true);
      if (Count == 2)
        CheckDlgButton(hDlg, ID_STYLESHEETFILESCHK, BST_CHECKED);
      else if (Count > 0)
        CheckDlgButton(hDlg, ID_STYLESHEETFILESCHK, BST_INDETERMINATE);
      else
        CheckDlgButton(hDlg, ID_STYLESHEETFILESCHK, BST_UNCHECKED);

      Count = IsAssociateTo(".js", true);
      if (Count == 1)
        CheckDlgButton(hDlg, ID_JAVASCRIPTFILESCHK, BST_CHECKED);
      else if (Count > 0)
        CheckDlgButton(hDlg, ID_JAVASCRIPTFILESCHK, BST_INDETERMINATE);
      else
        CheckDlgButton(hDlg, ID_JAVASCRIPTFILESCHK, BST_UNCHECKED);

      Count = IsAssociateTo(".php", true) + IsAssociateTo(".php4", true);
      if (Count == 2)
        CheckDlgButton(hDlg, ID_PHPFILESCHK, BST_CHECKED);
      else if (Count > 0)
        CheckDlgButton(hDlg, ID_PHPFILESCHK, BST_INDETERMINATE);
      else
        CheckDlgButton(hDlg, ID_PHPFILESCHK, BST_UNCHECKED);
      return TRUE;
    }
    case WM_NOTIFY:
    {
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_APPLY:
        {
          /* Save file associations*/
          AssociateTo(".lpg", true);
          if (IsDlgButtonChecked(hDlg, ID_TEXTFILESCHK) == BST_CHECKED)
          {
            AssociateTo(".txt", true);
            AssociateTo(".ini", true);
            AssociateTo(".inf", true);
            AssociateTo(".log", true);
          }
          else if (IsDlgButtonChecked(hDlg, ID_TEXTFILESCHK) == BST_UNCHECKED)
          {
            RemoveAssociation(".txt", true);
            RemoveAssociation(".ini", true);
            RemoveAssociation(".inf", true);
            RemoveAssociation(".log", true);
          }
          if (IsDlgButtonChecked(hDlg, ID_HTMLFILESCHK) == BST_CHECKED)
          {
            AssociateTo(".html", false);
            AssociateTo(".htm", false);
            AssociateTo(".htt", true);
            AssociateTo(".shtml", false);
            AssociateTo(".xht", false);
            AssociateTo(".xhtml", false);
            AssociateTo(".xml", false);
          }
          else if (IsDlgButtonChecked(hDlg, ID_HTMLFILESCHK) == BST_UNCHECKED)
          {
            RemoveAssociation(".html", false);
            RemoveAssociation(".htm", false);
            RemoveAssociation(".htt", true);
            RemoveAssociation(".shtml", false);
            RemoveAssociation(".xht", false);
            RemoveAssociation(".xhtml", false);
            RemoveAssociation(".xml", false);
          }
          if (IsDlgButtonChecked(hDlg, ID_STYLESHEETFILESCHK) == BST_CHECKED)
          {
            AssociateTo(".css", true);
            AssociateTo(".xsl", false);
          }
          else if (IsDlgButtonChecked(hDlg, ID_STYLESHEETFILESCHK) == BST_UNCHECKED)
          {
            RemoveAssociation(".css", true);
            RemoveAssociation(".xsl", false);
          }
          if (IsDlgButtonChecked(hDlg, ID_JAVASCRIPTFILESCHK) == BST_CHECKED)
          {
            AssociateTo(".js", true);
          }
          else if (IsDlgButtonChecked(hDlg, ID_JAVASCRIPTFILESCHK) == BST_UNCHECKED)
          {
            RemoveAssociation(".js", true);
          }
          if (IsDlgButtonChecked(hDlg, ID_PHPFILESCHK) == BST_CHECKED)
          {
            AssociateTo(".php", true);
            AssociateTo(".php4", true);
          }
          else if (IsDlgButtonChecked(hDlg, ID_PHPFILESCHK) == BST_UNCHECKED)
          {
            RemoveAssociation(".php", true);
            RemoveAssociation(".php4", true);
          }
          return TRUE;
        }
      }
      return FALSE;
    }
    default:
      return FALSE;
  }
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

#define NUMPAGES 5
void ShowPreferencesDialog()
{
  PROPSHEETPAGE PropSheet[NUMPAGES];
  PROPSHEETHEADER PropHeader;

  PropSheet[0].dwSize = sizeof(PROPSHEETPAGE);
  PropSheet[0].dwFlags = PSP_USETITLE;
  PropSheet[0].hInstance = hInstance;
  PropSheet[0].pszTemplate = MAKEINTRESOURCE(ID_PREFERENCESDIALOG1);
  PropSheet[0].pfnDlgProc = (DLGPROC)PreferencesDialog1Proc;
  PropSheet[0].lParam = 0;
  PropSheet[0].pszTitle = "General";
  PropSheet[0].pfnCallback = NULL;

  PropSheet[1].dwSize = sizeof(PROPSHEETPAGE);
  PropSheet[1].dwFlags = PSP_USETITLE;
  PropSheet[1].hInstance = hInstance;
  PropSheet[1].pszTemplate = MAKEINTRESOURCE(ID_PREFERENCESDIALOG2);
  PropSheet[1].pfnDlgProc = (DLGPROC)PreferencesDialog2Proc;
  PropSheet[1].lParam = 0;
  PropSheet[1].pszTitle = "Editor";
  PropSheet[1].pfnCallback = NULL;

  PropSheet[2].dwSize = sizeof(PROPSHEETPAGE);
  PropSheet[2].dwFlags = PSP_USETITLE;
  PropSheet[2].hInstance = hInstance;
  PropSheet[2].pszTemplate = MAKEINTRESOURCE(ID_PREFERENCESDIALOG3);
  PropSheet[2].pfnDlgProc = (DLGPROC)PreferencesDialog3Proc;
  PropSheet[2].lParam = 0;
  PropSheet[2].pszTitle = "Display";
  PropSheet[2].pfnCallback = NULL;

  PropSheet[3].dwSize = sizeof(PROPSHEETPAGE);
  PropSheet[3].dwFlags = PSP_USETITLE;
  PropSheet[3].hInstance = hInstance;
  PropSheet[3].pszTemplate = MAKEINTRESOURCE(ID_PREFERENCESDIALOG4);
  PropSheet[3].pfnDlgProc = (DLGPROC)PreferencesDialog4Proc;
  PropSheet[3].lParam = 0;
  PropSheet[3].pszTitle = "Colors";
  PropSheet[3].pfnCallback = NULL;

  PropSheet[4].dwSize = sizeof(PROPSHEETPAGE);
  PropSheet[4].dwFlags = PSP_USETITLE;
  PropSheet[4].hInstance = hInstance;
  PropSheet[4].pszTemplate = MAKEINTRESOURCE(ID_PREFERENCESDIALOG5);
  PropSheet[4].pfnDlgProc = (DLGPROC)PreferencesDialog5Proc;
  PropSheet[4].lParam = 0;
  PropSheet[4].pszTitle = "File associations";
  PropSheet[4].pfnCallback = NULL;

  PropHeader.dwSize = sizeof(PROPSHEETHEADER);
  PropHeader.dwFlags = PSH_PROPSHEETPAGE;
  PropHeader.hwndParent = hWindow;
  PropHeader.hInstance = hInstance;
  PropHeader.pszCaption = "Preferences";
  PropHeader.nPages = NUMPAGES;
  PropHeader.nStartPage = 0;
  PropHeader.ppsp = (LPCPROPSHEETPAGE)&PropSheet;
  PropertySheet(&PropHeader);
}
