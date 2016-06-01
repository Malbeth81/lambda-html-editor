#include "tools.h"

#define OPF_PRINTERNAME 0x01
#define OPF_PATHNAME 0x02
    
typedef void (__stdcall *pSHObjectProperties)(HWND, UINT, LPCSTR, LPCSTR);
    
int ArgumentCount(const char* Str)
{
  int Result = 0;
  int i = 0;
  while (i != -1 && i < (int)strlen(Str))
  {
    if (Str[i] == '"')
    {
      i = strpos(Str, '"', i+1);
      if (i != -1)
        i ++;
      Result ++;
    }
    else if (Str[i] == ' ')
      i = strnpos(Str, ' ', i);
    else
    {
      i = strpos(Str, ' ', i);
      Result ++;
    }
  }
  return Result;
}

HBRUSH CreateHollowBrush()
{
  LOGBRUSH Brush;
  Brush.lbStyle = BS_HOLLOW;
  return CreateBrushIndirect(&Brush);
}

HFONT EasyCreateFont(HDC DC, PFont Font)
{
  return EasyCreateFont(DC, Font->Name, Font->Size, Font->Style);
}

HFONT EasyCreateFont(HDC DC, char* FontName, int Size, TFontStyle Style)
{
  return CreateFont(-MulDiv(Size, GetDeviceCaps(DC, LOGPIXELSY), 72),
         0, 0, 0, (Style & fsBold) == fsBold ? FW_BOLD : FW_NORMAL,
         (Style & fsItalic) == fsItalic, (Style & fsUnderline) == fsUnderline, false,
         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
         DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FontName);
}

static HWND PreviousInstance = NULL;
BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
  char ClassName[128];
  GetClassName(hWnd, ClassName, 128);
  if (!lstrcmp(ClassName, (const char*)lParam))
  {
    PreviousInstance = hWnd;
    return FALSE;
  }
  return TRUE;
}

HWND FindPreviousInstance(const char* ClassName)
{
  EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)ClassName);
  return PreviousInstance;
}

char* FormatDate(SYSTEMTIME Time, int Flag, const char* Format)
{
  char* Result = new char[MAX_PATH];
  /* Get Timezone information */
  TIME_ZONE_INFORMATION TimeZone;
  GetTimeZoneInformation(&TimeZone);
  /* Apply timezone to the file time */
  Time.wHour = Time.wHour-(WORD)((int)TimeZone.Bias/60);
  Time.wMinute = Time.wMinute-(WORD)((int)TimeZone.Bias%60);
  /* Format to string */
  GetDateFormat(LOCALE_USER_DEFAULT, Flag, &Time, Format, Result, MAX_PATH);
  return Result;
}

char* FormatTime(SYSTEMTIME Time, int Flag, const char* Format)
{
  char* Result = new char[MAX_PATH];
  /* Get Timezone information */
  TIME_ZONE_INFORMATION TimeZone;
  GetTimeZoneInformation(&TimeZone);
  /* Apply timezone to the file time */
  Time.wHour = Time.wHour-(WORD)((int)TimeZone.Bias/60);
  Time.wMinute = Time.wMinute-(WORD)((int)TimeZone.Bias%60);
  /* Format to string */
  GetTimeFormat(LOCALE_USER_DEFAULT, Flag, &Time, Format, Result, MAX_PATH);
  return Result;
}

char* FormatSize(long Size)
{
  char* Result = new char[MAX_PATH];
  Result[0] = '\0';
  if (Size >= 1073741824)
  {
    char* Str = IntToStr(Size/1073741824);
    strcpy(Result, Str);
    delete[] Str;
    strcat(Result, ".");
    Str = IntToStr(Size%1073741824);
    strncat(Result, Str, 2);
    delete[] Str;
    strcat(Result, " GB (");
  }
  else if (Size >= 1048576)
  {
    char* Str = IntToStr(Size/1048576);
    strcpy(Result, Str);
    delete[] Str;
    strcat(Result, ".");
    Str = IntToStr(Size%1048576);
    strncat(Result, Str, 2);
    delete[] Str;
    strcat(Result, " MB (");
  }
  else if (Size >= 1024)
  {
    char* Str = IntToStr(Size/1024);
    strcpy(Result, Str);
    delete[] Str;
    strcat(Result, ".");
    Str = IntToStr(Size%1024);
    strncat(Result, Str, 2);
    delete[] Str;
    strcat(Result, " KB (");
  }
  char* Str = IntToStr(Size);
  strcat(Result, Str);
  delete[] Str;
  strcat(Result, " bytes");
  if (Size >= 1024)
    strcat(Result, ")");
  return Result;
}

bool GetFileCreationDate(const char* FileName, FILETIME* FileTime)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    *FileTime = Data.ftCreationTime;
    return true;
  }
  return false;
}

bool GetFileModificationDate(const char* FileName, FILETIME* FileTime)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    *FileTime = Data.ftLastWriteTime;
    return true;
  }
  return false;
}

long GetFileSize(const char* FileName)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    return (Data.nFileSizeHigh*MAXDWORD) + Data.nFileSizeLow;
  }
  return -1;
}

char* GetWindowText(HWND hWnd)
{
  int Len = GetWindowTextLength(hWnd);
  char* Str = new char[Len+1];
  GetWindowText(hWnd, Str, Len+1);
  return Str;
}

char* ColorToHTML(COLORREF Color)
{
  char* Result = new char[12];
  BYTE Red = GetRValue(Color);
  BYTE Green = GetGValue(Color);
  BYTE Blue = GetBValue(Color);
  strcpy(Result, "#");
  char Str[3];
  /* Red */
  if (Red <= 16)
    strcat(Result, "0");
  itoa(Red, Str, 16);
  strncat(Result, Str, 2);
  /* Green */
  if (Green <= 16)
    strcat(Result, "0");
  itoa(Green, Str, 16);
  strncat(Result, Str, 2);
  /* Blue */
  if (Blue <= 16)
    strcat(Result, "0");
  itoa(Blue, Str, 16);
  strncat(Result, Str, 2);
  return toUpper(Result);
}

COLORREF HTMLToColor(const char* Str)
{
  if (strlen(Str) >= 6)
  {
    if (Str[0] == '#')
      Str = Str+1;
    char Red[3];
    char Green[3];
    char Blue[3];
    strncpy(Red, Str, 2);
    Red[2] = '\0';
    strncpy(Green, Str+2, 2);
    Green[2] = '\0';
    strncpy(Blue, Str+4, 2);
    Blue[2] = '\0';
    return RGB(strtol(Red, NULL, 16), strtol(Green, NULL, 16), strtol(Blue, NULL, 16));
  }
  return 0;
}

char* IntToStr(long Number)
{
  char* Str = new char[12];
  itoa(Number, Str, 10);
  return Str;
}

bool IsDirectory(const char* FileName)
{
  WIN32_FIND_DATA Data;
  HANDLE hFind = FindFirstFile(FileName, &Data);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    return (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
  return false;
}

long LineBreakCount(const string Str)
{
  bool LineBreak;
  int Pos = 0;
  int Result = 0;
  do
  {
    Pos = Str.find("\r\n", Pos);
    if (Pos >= 0)
      LineBreak = true;
    else
    {
      Pos = Str.find_first_of("\r\n", Pos);
      Pos = (Pos < 0 ? Str.size() : Pos);
      LineBreak = false;
    }
    Result ++;
    Pos = Pos + (LineBreak ? 2 : 1);
  }
  while (Pos <= (int)Str.size());
  return Result;
}

long Max(long a, long b)
{
  return (a > b) ? a : b;
}

long Min(long a, long b)
{
  return (a < b) ? a : b;
}

PStringList OpenDialog(HWND Parent, const char* DefExt, const char* Filters)
{
  PStringList Result = new TStringList();
  char* FileName = new char[MAX_PATH*2];
  FileName[0] = '\0';
  OPENFILENAME File;
  File.lStructSize = sizeof(OPENFILENAME);
  File.hwndOwner = Parent;
  File.lpstrFilter = Filters;
  File.lpstrCustomFilter = NULL;
  File.nFilterIndex = 1;
  File.lpstrFile = FileName;
  File.nMaxFile = MAX_PATH*2;
  File.lpstrFileTitle = NULL;
  File.lpstrInitialDir = NULL;
  File.lpstrTitle = NULL;
  File.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
  File.lpstrDefExt = DefExt;
  File.lCustData = 0;
  if (GetOpenFileName(&File) != 0)
  {
    if (GetFileAttributes(File.lpstrFile) & FILE_ATTRIBUTE_DIRECTORY)
    {
      char* Ptr = File.lpstrFile + File.nFileOffset;
      do
      {
        Result->Add(File.lpstrFile);
        (*Result)[Result->Count()-1].append(1, '\\');
        (*Result)[Result->Count()-1].append(Ptr);
        Ptr += strlen(Ptr)+1;
      }
      while (*Ptr != '\0');
    }
    else
      Result->Add(File.lpstrFile);
  }
  delete[] FileName;
  return Result;
}

bool OpenFileAs(const char* FileName)
{
  SHELLEXECUTEINFO info;

  memset(&info, 0, sizeof(info));
  info.cbSize = sizeof(info);
  info.fMask = SEE_MASK_NOCLOSEPROCESS;
  info.lpVerb = "openas";
  info.lpFile = FileName;
  info.nShow = SW_SHOWNORMAL;

  if (!ShellExecuteEx(&info))
    return false;
  return true;
}

bool OpenFileProperties(const char* FileName)
{
  BOOL (WINAPI*SHObjectProperties)(HWND,UINT,LPCSTR,LPCSTR);
  SHObjectProperties = (BOOL (WINAPI*)(HWND,UINT,LPCSTR,LPCSTR))GetProcAddress(GetModuleHandle("SHELL32.DLL"), (LPCSTR)178);
  (*SHObjectProperties)(NULL, OPF_PATHNAME, "c:\\a.txt", NULL);
}

void ParseArguments(const char* Str, string Args[])
{
  bool Quote = false;
  int i = 0;
  int j = 0;
  while (Str[i] != '\0')
  {
    if (Str[i] == ' ' && !Quote)
    {
      i = strnpos(Str, ' ', i);
      j++;
    }
    else if (Str[i] == '"')
    {
      Quote = !Quote;
      i++;
    }
    else
    {
      Args[j].append(&Str[i], 1);
      i++;
    }
  }
}

TPoint Point(long x, long y)
{
  TPoint Pt;
  Pt.x = x;
  Pt.y = y;
  return Pt;
}

int PointCmp(TPoint a, TPoint b)
{
  if (a.y == b.y)
    return a.x - b.x;
  else
    return a.y - b.y;
}

HDC PrintDialog(HWND Parent)
{
  PRINTDLG DlgInfo;
  DlgInfo.lStructSize = sizeof(PRINTDLG);
  DlgInfo.hwndOwner = Parent;
  DlgInfo.hDevMode = NULL;
  DlgInfo.hDevNames = NULL;
  DlgInfo.Flags = PD_ALLPAGES | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
  DlgInfo.nCopies = 1;
  PrintDlg(&DlgInfo);
  return DlgInfo.hDC;
}

RECT Rect(const int Left, const int Top, const int Right, const int Bottom)
{
  RECT R;
  R.left = Left;
  R.top = Top;
  R.bottom = Bottom;
  R.right = Right;
  return R;
}

bool SaveDialog(HWND Parent, string InitialDir, string* DefFileName, const char* DefExt, const char* Filters, const int FilterIndex)
{
  bool Result = false;
  char* FileName = new char[MAX_PATH*2];
  strcpy(FileName, DefFileName->c_str());
  OPENFILENAME File;
  File.lStructSize = sizeof(OPENFILENAME);
  File.hwndOwner = Parent;
  File.lpstrFilter = Filters;
  File.lpstrCustomFilter = NULL;
  File.nFilterIndex = FilterIndex;
  File.lpstrFile = FileName;
  File.nMaxFile = MAX_PATH*2;
  File.lpstrFileTitle = NULL;
  File.lpstrInitialDir = InitialDir.c_str();
  File.lpstrTitle = NULL;
  File.Flags = OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
  File.lpstrDefExt = DefExt;
  File.lCustData = 0;
  if (GetSaveFileName(&File) != 0)
  {
    DefFileName->assign(FileName);
    Result = true;
  }
  delete[] FileName;
  return Result;
}

char* Spaces(int Count)
{
  char* Str = new char[Count+1];
  for (int i = 0; i < Count; i++)
    Str[i] = ' ';
  Str[Count] = '\0';
  return Str;
}

int strpos(const char* Str, const char Char, const int Index)
{
  bool Found = false;
  int i = Index;
  while (Str[i] != '\0')
    if (Str[i] == Char)
    {
      Found = true;
      break;
    }
    else
      i++;
  if (Found)
    return i;
  else
    return -1;
}

int strpos(char* Str, const char* Str2, const int Index)
{
  bool Found = false;
  int i = Index;
  while (Str[0] != '\0')
    if (strncmp(Str, Str2, strlen(Str2)) == 0)
    {
      Found = true;
      break;
    }
    else
    {
      Str += 1;
      i++;
    }
  if (Found)
    return i;
  else
    return -1;
}

int stripos(const char* Str, const char* Str2, int Index)
{
  int Size = strlen(Str);
  int Size2 = strlen(Str2);
  for (int i = Index; i < Size-Size2+1; i++)
    if (strnicmp(Str+i, Str2, Size2) == 0)
      return i;
  return -1;
}

int strnpos(const char* Str, const char Char, const int Index)
{
  bool Found = false;
  int i = Index;
  while (Str[i] != '\0')
    if (Str[i] != Char)
    {
      Found = true;
      break;
    }
    else
      i++;
  if (Found)
    return i;
  else
    return -1;
}

int strrpos(const char* Str, const char Char, const int Index)
{
  bool Found = false;
  int i = strlen(Str)-1-Index;
  while (i > 0)
    if (Str[i] == Char)
    {
      Found = true;
      break;
    }
    else
      i--;
  if (Found)
    return i;
  else
    return -1;
}

char* toLower(const char* Str)
{
  int length = strlen(Str);
  char* Result = new char[length+1];
  strcpy(Result, Str);
  for (int i = 0; i < length; i++)
    Result[i] = (char)tolower(Str[i]);
  return Result;
}

char* toUpper(const char* Str)
{
  int length = strlen(Str);
  char* Result = new char[length+1];
  strcpy(Result, Str);
  for (int i = 0; i < length; i++)
    Result[i] = (char)toupper(Str[i]);
  return Result;
}

bool GetPrivateProfileBool(const char* Section, const char* Key, bool Default, const char* FileName)
{
  char* Str = new char[32];
  GetPrivateProfileString(Section, Key, (Default ? "true" : "false"), Str, 32, FileName);
  if (strcmp(Str, "true") == 0)
    return true;
  return false;
}

BOOL WritePrivateProfileBool(const char* Section, const char* Key, bool Value, const char* FileName)
{
  const char* Str = (Value ? "true" : "false");
  return WritePrivateProfileString(Section, Key, Str, FileName);
}

BOOL WritePrivateProfileInt(const char* Section, const char* Key, int Value, const char* FileName)
{
  char* Str = IntToStr(Value);
  BOOL Result = WritePrivateProfileString(Section, Key, Str, FileName);
  delete[] Str;
  return Result;
}
