#ifndef TOOLS_H
#define	TOOLS_H

#include <windows.h>
#include <string>
using namespace std;
#include "models/stringlist.h"

#define fsBold      1
#define fsItalic    2
#define fsUnderline 4

typedef unsigned short TFontStyle;
typedef struct TFont {
    char Name[LF_FACESIZE];
    int Size;
    TFontStyle Style;
} *PFont;

typedef struct TPoint {
  long x;
  long y;
} *PPoint;

int ArgumentCount(const char* Str);
char* ColorToHTML(COLORREF Color);
HBRUSH CreateHollowBrush();
HFONT EasyCreateFont(HDC DC, TFont* Font);
HFONT EasyCreateFont(HDC DC, char* FontName, int Size, TFontStyle Style);
HWND FindPreviousInstance(const char* ClassName);
char* FormatDate(SYSTEMTIME Time, int Flag, const char* Format);
char* FormatTime(SYSTEMTIME Time, int Flag, const char* Format);
char* FormatSize(long Size);
bool GetFileCreationDate(const char* FileName, FILETIME* FileTime);
bool GetFileModificationDate(const char* FileName, FILETIME* FileTime);
long GetFileSize(const char* FileName);
char* GetWindowText(HWND hWnd);
COLORREF HTMLToColor(const char* Color);
char* IntToStr(long Number);
bool IsDirectory(const char* FileName);
long Max(long a, long b);
long Min(long a, long b);
PStringList OpenDialog(HWND Parent, const char* DefExt, const char* Filters);
bool OpenFileAs(const char* FileName);
bool OpenFileProperties(const char* FileName);
void ParseArguments(const char* Str, string Args[]);
TPoint Point(long x, long y);
int PointCmp(TPoint a, TPoint b);
HDC PrintDialog(HWND Parent);
RECT Rect(const int Left, const int Top, const int Right, const int Bottom);
bool SaveDialog(HWND Parent, string InitialDir, string* DefFileName, const char* DefExt, const char* Filters, const int FilterIndex = 1);
char* Spaces(int Count);
int strpos(const char* Str, const char Char, const int Index = 0);
int strpos(char* Str, const char* Str2, const int Index = 0);
int stripos(const char* Str, const char* Str2, int Index = 0);
int strnpos(const char* Str, const char Char, const int Index = 0);
int strrpos(const char* Str, const char Char, const int Index = 0);
//void toLower(string& Str);
char* toLower(const char* Str);
char* toUpper(const char* Str);
bool GetPrivateProfileBool(const char* Section, const char* Key, bool Default, const char* FileName);
BOOL WritePrivateProfileBool(const char* Section, const char* Key, bool Value, const char* FileName);
BOOL WritePrivateProfileInt(const char* Section, const char* Key, int Value, const char* FileName);

#endif
