#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "main.h"
#include "menu.h"
#include "controls/splitter.h"
#include "controls/tabs.h"
#include "controls/toolbar.h"

const char ClassName[] = "Lambda";
const char WindowTitle[] = "Lambda HTML Editor";

HINSTANCE hInstance = NULL;
HANDLE hMutex = NULL;
HWND hWindow = NULL;
HWND hToolbar = NULL;
HWND hTabs = NULL;
HWND hSplitter = NULL;
HWND hStatusbar = NULL;

LambdaEditor* Editor = NULL;
ProjectExplorer* Explorer;

bool AllowMultipleInstance = false;
bool AutoConvertChars = true;
bool AutoUpdate = false;
bool OpenUnsupportedFiles = false;
bool DefaultInternetBrowser = true;
string InternetBrowser = "";


#define AllSupportedFilesFilter "All Supported Files\0*.lpg;*.txt;*.ini;*.inf;*.log;*.cfg;*.html;*.htm;*.htt;*.htx;*.shtml;*.xht;*.xhtml;*.xml;*.css;*.xsl;*.js;*.php;*.php4;*.h;*.c;*.cpp;*.rc;*.pas;*.dfm;*.dof;*.dpr;*.manifest;*.dtd;*.ada;*.adb;*.ads;*.an;*.ans;*.asc;*.csa;*.jw;*.me;*.mss;*.info;*.nfo;*.pc;*.pcw;*.unx;*.xy\0"
#define AllSupportedDocsFilter "All Supported Documents\0*.txt;*.ini;*.inf;*.log;*.cfg;*.html;*.htm;*.htt;*.htx;*.shtml;*.xht;*.xhtml;*.xml;*.css;*.xsl;*.js;*.php;*.php4;*.h;*.c;*.cpp;*.rc;*.pas;*.dfm;*.dof;*.dpr;*.manifest;*.dtd;*.ada;*.adb;*.ads;*.an;*.ans;*.asc;*.csa;*.jw;*.me;*.mss;*.info;*.nfo;*.pc;*.pcw;*.unx;*.xy\0"
#define ProjectFilter "Lambda Editor Project (*.lpg)\0*.lpg\0"
#define TextFileFilter "Text File (*.txt, *.ini, *.inf, *.log)\0*.txt;*.ini;*.inf;*.log\0"
#define HTMLDocumentFilter "HTML Document (*.html, *.htt, *.shtml, *.xhtml, *.xml)\0*.html;*.htm;*.htt;*.shtml;*.xht;*.xhtml;*.xml\0"
#define HTMLStylesheetFilter "HTML Stylesheet (*.css, *.xsl)\0*.css;*.xsl\0"
#define JavaScriptUnitFilter "JavaScript Unit (*.js)\0*.js\0"
#define PHPDocumentFilter "PHP Document (*.php, *.php4)\0*.php;*.php4\0"
#define AllFilesFilter "All Files (*.*)\0*.*\0\0"

const char* SupportedFilesExt[SupportedFilesExtCount] = {"txt","ini","inf","log",
"cfg","html","htm","htt","htx","shtml","xht","xhtml","xml","css","xsl","js","php",
"php4","h","c","cpp","rc","pas","dfm","dof","dpr","manifest","dtd","ada","adb",
"ads","an","ans","asc","csa","jw","me","mss","info","nfo","pc","pcw","unx","xy"};

const char* SingleTags[SingleTagsCount] = {"area","base","br","col","frame",
"hr","img","input","link","meta","param"};

const char* PairedTags[PairedTagsCount] = {"a","abbr","acronym","address","b",
"bdo","big","blockquote","body","button","caption","cite","code","colgroup",
"dd","del","dfn","div","dl","dt","em","fieldset","form","frameset","h1","h2",
"h3","h4","h5","h6","head","html","i","iframe","ins","kdb","label","legend",
"li","map","noframes","noscript","object","ol","optgroup","option","p","pre",
"q","samp","script","select","small","span","strong","style","sub","sup","table",
"tbody","td","textarea","tfoot","th","thead","title","tr","tt","ul","var"};

const char* Characters[CharactersCount] = {"<",">","&"," ","\"","�","�","�","�",
"�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�",
"�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�",
"�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�",
"�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�","�",
"�","�","�","�","�","�","�","�","�","�","�","�"};

const char* CharCodes[CharactersCount] = {"&lt;","&gt;","&amp;","&nbsp;","&quot;",
"&iexcl;","&cent;","&pound;","&curren;","&yen;","&brvbar;","&sect;","&uml;",
"&copy;","&ordf;","&laquo;","&not;","&shy;","&reg;","&#8482;","&macr;","&deg;",
"&plusmn;","&sup2;","&sup3;","&acute;","&micro;","&para;","&middot;","&cedil;",
"&sup1;","&ordm;","&raquo;","&frac14;","&frac12;","&frac34;","&iquest;","&Agrave;",
"&Aacute;","&Acirc;","&Atilde;","&Auml;","&Aring;","&AElig;","&Ccedil;","&Egrave;",
"&Eacute;","&Ecirc;","&Euml;","&Igrave;","&Iacute;","&Icirc;","&Iuml;","&ETH;",
"&Ntilde;","&Ograve;","&Oacute;","&Ocirc;","&Otilde;","&Ouml;","&times;","&Oslash;",
"&Ugrave;","&Uacute;","&Ucirc;","&Uuml;","&Yacute;","&THORN;","&szlig;","&agrave;",
"&aacute;","&acirc;","&atilde;","&auml;","&aring;","&aelig;","&ccedil;","&egrave;",
"&eacute;","&ecirc;","&euml;","&igrave;","&iacute;","&icirc;","&iuml;","&eth;",
"&ntilde;","&ograve;","&oacute;","&ocirc;","&otilde;","&ouml;","&divide;",
"&oslash;","&ugrave;","&uacute;","&ucirc;","&uuml;","&yacute;","&thorn;","&yuml;"};

// PRIVATE FUNCTIONS -----------------------------------------------------------

void AdjustSize()
{
  /* Obtain window size */
  RECT R = GetClientRect();
  /* Resize toolbar */
  if (hToolbar != NULL)
    SetWindowPos(hToolbar, NULL, R.left, 0, R.right, TOOLBAR_HEIGHT, SWP_NOZORDER);
  /* Resize explorer */
  if (Explorer != NULL)
    SetWindowPos(Explorer->GetHandle(), NULL, R.left, R.top, SplitterPos, R.bottom-R.top, SWP_NOZORDER);
  /* Resize splitter */
  if (Explorer != NULL && Explorer->IsVisible())
    R.left += SplitterPos;
  if (hSplitter != NULL)
  {
    if (HideSplitter != (IsWindowVisible(hSplitter) == 0))
      ShowWindow(hSplitter, HideSplitter ? SW_HIDE : SW_SHOW);
    if (!HideSplitter)
      SetWindowPos(hSplitter, NULL, R.left, R.top, 5, R.bottom-R.top, SWP_NOZORDER);
  }
  /* Resize tabs & editor */
  if (Explorer != NULL && !HideSplitter)
    R.left += 5;
  if (hTabs != NULL)
  {
    SetWindowPos(hTabs, NULL, R.left, R.top, R.right-R.left, R.bottom-R.top, SWP_NOZORDER);
    if (Editor != NULL)
    {
      TabCtrl_AdjustRect(hTabs, FALSE, &R);
      SetWindowPos(Editor->GetHandle(), NULL, R.left-1, R.top+1, R.right-R.left+2, R.bottom-R.top, 0);
    }
  }
}

bool FileIsSupported(const char* FileName)
{
  bool Result = false;
  int ExtPos = strrpos(FileName, '.');
  if (ExtPos >= 0)
  {
    char* Ext = new char[strlen(FileName)-ExtPos+1];
    strcpy(Ext, (char*)FileName+ExtPos+1);
    for (int i = 0; i < SupportedFilesExtCount; i++)
      if (stripos(SupportedFilesExt[i], Ext) >= 0)
        Result = true;
    delete[] Ext;
  }
  return Result;
}

void OpenCmdLineDocuments(const char* cmdLine)
{
  int ArgCount = ArgumentCount(cmdLine);
  if (ArgCount > 0)
  {
    string* Args = new string[ArgCount];
    ParseArguments(cmdLine, Args);
    for (int i = 0; i < ArgCount; i++)
      OpenFile(Args[i].c_str());
  }
}

void ReplaceAllChars(string* Str, char Char, const char* Value)
{
  int Index = Str->find(Char);
  while (Index >= 0)
  {
    Str->replace(Index, 1, Value);
    Index = Str->find(Char, Index+strlen(Value));
  }
}

// PRIVATE CALLBACK FUNCTIONS --------------------------------------------------

void __stdcall ActiveDocumentChanged()
{
  SetActiveTab(hTabs, Editor->DocumentIndex());
  UpdateStatusbar();
  UpdateToolbar();
}

void __stdcall CursorPosChanged()
{
  UpdateStatusbar();
  UpdateToolbar();
}

void __stdcall DocumentOpened(const char* FileName)
{
  if (strlen(FileName) == 0)
    AddTab(hTabs, "New document");
  else
    AddTab(hTabs, strrchr(FileName, '\\')+1);
  UpdateStatusbar();
  UpdateToolbar();
}

bool __stdcall DocumentClosed(int Index)
{
  if (Editor->GetDocument(Index)->IsModified())
  {
    string Str = Editor->GetDocument()->GetFileName();
    if (Str.size() == 0)
      Str = "New document";
    int Result = MessageBox(hWindow, ("The document named \""+Str+"\" has been modified, do you want to save it?").c_str(), "Closing a document", MB_YESNOCANCEL);
    if (Result == IDYES)
      SaveDocument(Index);
    else if (Result == IDCANCEL)
      return false;
  }
  RemoveTab(hTabs, Index);
  return true;
}

void __stdcall DocumentMoved(int FromIndex, int ToIndex)
{
  MoveTab(hTabs, FromIndex, ToIndex);
  Editor->MoveDocument(FromIndex, ToIndex);
}

void __stdcall DocumentRenamed(int Index, const char* FileName)
{
  if (strlen(FileName) == 0)
    EditTab(hTabs, Index, "New document");
  else
    EditTab(hTabs, Index, strrchr(FileName, '\\')+1);
  UpdateStatusbar();
}

void __stdcall StateChanged()
{
  UpdateStatusbar();
  UpdateToolbar();
}

void __stdcall StringInserted(string* Str)
{
  if (AutoConvertChars)
  {
//    Cursor := crHourglass;
    for (int i = 5; i < CharactersCount; i++)
      ReplaceAllChars(Str, Characters[i][0], CharCodes[i]);
//    Cursor := crDefault;
  }
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

void AddToProject()
{
  PStringList Files = OpenDialog(hWindow, DefaultFileExt, AllSupportedDocsFilter TextFileFilter HTMLDocumentFilter HTMLStylesheetFilter JavaScriptUnitFilter PHPDocumentFilter AllFilesFilter);
  for (int i = 0; i < Files->Count(); i++)
    Explorer->AddFile("", (*Files)[i].c_str());
  delete Files;
  UpdateToolbar();
}

void ChangeHTMLTagCase(bool UpperCase)
{
  if (Editor->GetDocument() != NULL)
  {
    char* TagName = new char[32];
    TPoint Caret = Editor->GetDocument()->GetCaretPos();
    for (int i = 0; i < SingleTagsCount; i++)
    {
      strcpy(TagName, "<");
      strcat(TagName, SingleTags[i]);
      Editor->GetDocument()->SetCaretPos(0, 0, false);
      ReplaceAll(TagName, (UpperCase ? toUpper(TagName) : TagName), false, true, false);
    }
    for (int i = 0; i < PairedTagsCount; i++)
    {
      strcpy(TagName, "<");
      strcat(TagName, PairedTags[i]);
      Editor->GetDocument()->SetCaretPos(0, 0, false);
      ReplaceAll(TagName, (UpperCase ? toUpper(TagName) : TagName), false, true, false);
      strcpy(TagName, PairedTags[i]);
      strcat(TagName, ">");
      Editor->GetDocument()->SetCaretPos(0, 0, false);
      ReplaceAll(TagName, (UpperCase ? toUpper(TagName) : TagName), false, true, false);
    }
    Editor->GetDocument()->SetCaretPos(Caret.x, Caret.y, false);
  }
}

bool Find(const char* FindWhat, bool FromCaret, bool Down, bool MatchCase, bool WholeWords)
{
  if (Editor->GetDocument() != NULL)
  {
    TPoint Pos = Editor->GetDocument()->FindString(FindWhat, FromCaret, Down, MatchCase, WholeWords);
    if (Pos.x >= 0 && Pos.y >= 0)
    {
      Editor->GetDocument()->SetCaretPos(Pos.x+strlen(FindWhat), Pos.y);
      Editor->GetDocument()->Select(Pos, strlen(FindWhat));
      return true;
    }
  }
  return false;
}

RECT GetClientRect()
{
  /* Obtain window size */
  RECT R;
  GetClientRect(hWindow, &R);
  /* Adjust the size according to toolbar */
  R.top += TOOLBAR_HEIGHT+1;
  /* Adjust the size according to statusbar */
  RECT Q;
  GetClientRect(hStatusbar, &Q);
  R.bottom -= Q.bottom;
  return R;
}

void LoadSettings()
{
  char* Filename = new char[MAX_PATH];
  GetModuleFileName(hInstance, Filename, MAX_PATH);
  int Pos = strrpos(Filename, '\\');
  if (Pos >= 0)
    Filename[Pos+1] = '\0';
  strcat(Filename, "settings.ini");
  char* Str = new char[MAX_PATH];

  AllowMultipleInstance = GetPrivateProfileBool("Lambda", "AllowMultipleInstance", false, Filename);
  AutoConvertChars = GetPrivateProfileBool("Lambda", "AutoConvertChars", true, Filename);
  AutoUpdate = GetPrivateProfileBool("Lambda", "AutomaticlyUpdate", false, Filename);
  OpenUnsupportedFiles = GetPrivateProfileBool("Lambda", "OpenUnsupportedFiles", false, Filename);
  DefaultInternetBrowser = GetPrivateProfileBool("Lambda", "UseDefaultInternetBrowser", true, Filename);
  GetPrivateProfileString("Lambda", "InternetBrowser", "iexplore.exe", Str, MAX_PATH, Filename);
  InternetBrowser = Str;
  Explorer->SetHide(GetPrivateProfileBool("Lambda", "HideExplorer", false, Filename));
  HideSplitter = GetPrivateProfileBool("Lambda", "HideSplitter", false, Filename);

  int Left = GetPrivateProfileInt("Lambda", "Left", 0, Filename);
  int Top = GetPrivateProfileInt("Lambda", "Top", 0, Filename);
  int Height = GetPrivateProfileInt("Lambda", "Height", 400, Filename);
  int Width = GetPrivateProfileInt("Lambda", "Width", 800, Filename);
  MoveWindow(hWindow, Left, Top, Width, Height, FALSE);

  Explorer->SingleClickOpen = GetPrivateProfileBool("Project Explorer", "OpenOnSingleClick", false, Filename);
  Explorer->RenameOnClick = GetPrivateProfileBool("Project Explorer", "RenameOnClick", false, Filename);
  Explorer->SetAutoHide(GetPrivateProfileBool("Project Explorer", "AutomaticlyHide", false, Filename));
  Explorer->AutoHideDelay = 1000*GetPrivateProfileInt("Project Explorer", "AutomaticHideDelay", 2, Filename);
  SplitterPos = GetPrivateProfileInt("Project Explorer", "Width", 200, Filename);

  Editor->OverwriteEmptyDocs = GetPrivateProfileBool("Editor", "OverwriteEmptyDocuments", true, Filename);
  Editor->OverwriteSelection = GetPrivateProfileBool("Editor", "OverwriteSelectedText", true, Filename);
  Editor->OverwriteMode = GetPrivateProfileBool("Editor", "OverwriteTypingMode", false, Filename);
  Editor->AutoIndent = GetPrivateProfileBool("Editor", "IndentAutomaticly", true, Filename);
  Editor->GroupUndo = GetPrivateProfileBool("Editor", "GroupUndos", false, Filename);
  Editor->DoubleClickLine = GetPrivateProfileBool("Editor", "SelectLineOnDoubleClick", false, Filename);
  Editor->SpaceSeparatedWords = GetPrivateProfileBool("Editor", "SpaceSeparatedWords", false, Filename);
  Editor->TabIndentsLines = GetPrivateProfileBool("Editor", "TabIndentsLines", true, Filename);
  Editor->TabSize = GetPrivateProfileInt("Editor", "TabSize", 4, Filename);
  Editor->DefaultScrollLineCount = GetPrivateProfileBool("Editor", "UseDefaultScrollLineCount", true, Filename);
  Editor->ScrollLineCount = GetPrivateProfileInt("Editor", "ScrollLineCount", 3, Filename);
  Editor->SetGutterVisible(GetPrivateProfileBool("Editor", "GutterVisible", true, Filename));
  Editor->SetMarginVisible(GetPrivateProfileBool("Editor", "MarginVisible", true, Filename));
  Editor->SetLineNumbersVisible(GetPrivateProfileBool("Editor", "LineNumbersVisible", true, Filename));
  Editor->SetAutoSizeGutter(GetPrivateProfileBool("Editor", "AutomaticlyResizeGutter", true, Filename));
  Editor->SetGutterSize(GetPrivateProfileInt("Editor", "GutterSize", 22, Filename));
  Editor->SetMarginSize(GetPrivateProfileInt("Editor", "MarginSize", 80, Filename));
  TFont Font;
  Font.Style = 0;
  GetPrivateProfileString("Editor", "EditorFontName", "Courier New", Str, MAX_PATH, Filename);
  strncpy(Font.Name, Str, LF_FACESIZE);
  Font.Size = GetPrivateProfileInt("Editor", "EditorFontSize", 10, Filename);
  Editor->SetFont(Font);
  GetPrivateProfileString("Editor", "GutterFontName", "Courier New", Str, MAX_PATH, Filename);
  strncpy(Font.Name, Str, LF_FACESIZE);
  Font.Size = GetPrivateProfileInt("Editor", "GutterFontSize", 10, Filename);
  Editor->SetGutterFont(Font);
  delete[] Str;
}

void Open()
{
  PStringList Files = OpenDialog(hWindow, DefaultFileExt, AllSupportedFilesFilter ProjectFilter TextFileFilter HTMLDocumentFilter HTMLStylesheetFilter JavaScriptUnitFilter PHPDocumentFilter AllFilesFilter);
  for (int i = 0; i < Files->Count(); i++)
    OpenFile((*Files)[i].c_str());
  delete Files;
}

void OpenDirectory(const char* FileName)
{
  WIN32_FIND_DATA FindData;
  char* Str = new char[MAX_PATH];
  strcpy(Str, FileName);
  strcat(Str, "\\*.*");
  HANDLE Handle = FindFirstFile(Str, &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.')
    {
      char* Str2 = new char[MAX_PATH];
      strcpy(Str2, FileName);
      strcat(Str2, "\\");
      strcat(Str2, FindData.cFileName);
      if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        OpenDirectory(Str2);
      else
        OpenDocument(Str2);
      delete[] Str2;
    }
    while (FindNextFile(Handle, &FindData));
    FindClose(Handle);
  }
  delete[] Str;
}

void OpenDocument(const char* FileName)
{
  if (OpenUnsupportedFiles || FileIsSupported(FileName))
    Editor->LoadDocument(FileName);
  else if (MessageBox(hWindow, "The document you selected is not recognized to be a supported file format. Do you want to open it in the associated application instead?", "Opening a document", MB_YESNO) == IDYES)
    if ((int)ShellExecute(hWindow, "edit", FileName, NULL, NULL, SW_SHOWNORMAL) <= 32)
      if ((int)ShellExecute(hWindow, "open", FileName, NULL, NULL, SW_SHOWNORMAL) <= 32)
        OpenFileAs(FileName);
}

void OpenFile(const char* FileName)
{
  int ExtPos = strrpos(FileName, '.');
  if (ExtPos >= 0 && strcmp((char*)FileName+ExtPos+1, ProjectFileExt) == 0)
  {
    while (Editor->DocumentCount() > 0)
      Editor->CloseDocument(0);
    Explorer->LoadFromFile(FileName);
  }
  else
    OpenDocument(FileName);
}

void OpenInBrowser(const char* FileName)
{
  if (strlen(FileName) > 0)
  {
    if (DefaultInternetBrowser)
      ShellExecute(hWindow, "open", FileName, NULL, NULL, SW_SHOWNORMAL);
    else
      ShellExecute(hWindow, "open", InternetBrowser.c_str(), FileName, NULL, SW_SHOWNORMAL);
  }
}

void Quit()
{
  PostMessage(hWindow, WM_CLOSE, 0, 0);
}

void ReplaceSelection(const char* ReplaceWith)
{
  if (Editor->GetDocument() != NULL)
  {
    TPoint Start = Editor->GetDocument()->GetSelStart();
    TPoint End = Editor->GetDocument()->GetSelEnd();
    if (PointCmp(Start, End) != 0 && Start.y == End.y)
    {
      Editor->GetDocument()->ReplaceString(Start, End.x-Start.x, ReplaceWith);
      Editor->GetDocument()->SetCaretPos(Start.x+strlen(ReplaceWith), Start.y);
      Editor->GetDocument()->Select(Start, strlen(ReplaceWith));
    }
  }
}

void ReplaceAll(const char* FindWhat, const char* ReplaceWith, bool FromCaret, bool MatchCase, bool WholeWords)
{
  if (Editor->GetDocument() != NULL)
  {
    if (FromCaret == false)
      Editor->GetDocument()->SetCaretPos(0, 0, false);
    while (Find(FindWhat, true, true, MatchCase, WholeWords))
      ReplaceSelection(ReplaceWith);
  }
}

void SaveAll()
{
  for (int i = 0; i < Editor->DocumentCount(); i++)
    SaveDocument(i);
  SaveProject();
}

void SaveAsTemplate(int Index)
{
  /* Determine the document type */
  string FileName = Editor->GetDocument(Index)->GetFileName();
  const char* FileExt;
  int ExtPos = FileName.find_last_of('.');
  if (ExtPos >= 0)
    FileExt = FileName.substr(ExtPos+1).c_str();
  else
    FileExt = DefaultFileExt;
  int FilterIndex = Editor->GetDocument(Index)->GetType()-TEXT_DOCUMENT+1;
  /* Get the path to the template dir */
  char* Dir = new char[MAX_PATH];
  GetModuleFileName(hInstance, Dir, MAX_PATH);
  int Pos = strrpos(Dir, '\\');
  if (Pos >= 0)
    Dir[Pos+1] = '\0';
  strcat(Dir, "templates\\");
  /* Saves the document */
  FileName = FileName.substr(FileName.find_last_of('\\')+1);
  if (SaveDialog(hWindow, Dir, &FileName, FileExt, TextFileFilter HTMLDocumentFilter HTMLStylesheetFilter JavaScriptUnitFilter PHPDocumentFilter AllFilesFilter, FilterIndex))
  {
    Editor->GetDocument(Index)->SetFileName(FileName);
    SaveDocument(Index);
  }
  delete[] Dir;
}

void SaveDocument(int Index)
{
  if (Editor->GetDocument(Index) != NULL)
  {
    if (Editor->GetDocument(Index)->GetFileName().size() == 0)
      SaveDocumentAs(Index);
    else if (Editor->SaveDocument(Index) == false)
    {
      MessageBox(hWindow, ("An error occured while saving the document \""+Editor->GetDocument(Index)->GetFileName()+"\", the file may be marked as \"read only\" or the specified file name may be invalid. Please specify a different file name to save to.").c_str(), "Saving a document", MB_OK);
      SaveDocumentAs(Index);
    }
  }
}

void SaveDocumentAs(int Index)
{
  /* Determine the document type */
  string FileName = Editor->GetDocument(Index)->GetFileName();
  char* FileExt = new char[MAX_PATH];
  int ExtPos = FileName.find_last_of('.');
  if (ExtPos >= 0)
    strcpy(FileExt, FileName.substr(ExtPos+1).c_str());
  else
    FileExt = DefaultFileExt;
  int FilterIndex = Editor->GetDocument(Index)->GetType()-TEXT_DOCUMENT+1;
  /* Saves the document */
  if (SaveDialog(hWindow, "", &FileName, FileExt, TextFileFilter HTMLDocumentFilter HTMLStylesheetFilter JavaScriptUnitFilter PHPDocumentFilter AllFilesFilter, FilterIndex))
  {
    Editor->GetDocument(Index)->SetFileName(FileName);
    SaveDocument(Index);
  }
  delete[] FileExt;
}

void SaveProject()
{
  if (Explorer->GetFileName().empty())
    SaveProjectAs();
  else
    Explorer->SaveToFile(Explorer->GetFileName().c_str());
}

void SaveProjectAs()
{
  string FileName = Explorer->GetFileName();
  if (SaveDialog(hWindow, "", &FileName, ProjectFileExt, ProjectFilter AllFilesFilter))
    Explorer->SaveToFile(FileName.c_str());
}

void SaveSettings()
{
  RECT R;
  char* Filename = new char[MAX_PATH];
  GetModuleFileName(hInstance, Filename, MAX_PATH);
  int Pos = strrpos(Filename, '\\');
  if (Pos >= 0)
    Filename[Pos+1] = '\0';
  strcat(Filename, "settings.ini");
  WritePrivateProfileBool("Lambda", "AllowMultipleInstance", AllowMultipleInstance, Filename);
  WritePrivateProfileBool("Lambda", "AutoConvertChars", AutoConvertChars, Filename);
  WritePrivateProfileBool("Lambda", "AutomaticlyUpdate", AutoUpdate, Filename);
  WritePrivateProfileBool("Lambda", "OpenUnsupportedFiles", OpenUnsupportedFiles, Filename);
  WritePrivateProfileBool("Lambda", "UseDefaultInternetBrowser", DefaultInternetBrowser, Filename);
  WritePrivateProfileString("Lambda", "InternetBrowser", InternetBrowser.c_str(), Filename);
  WritePrivateProfileBool("Lambda", "HideExplorer", Explorer->GetHide(), Filename);
  WritePrivateProfileBool("Lambda", "HideSplitter", HideSplitter, Filename);

  GetWindowRect(hWindow, &R);
  WritePrivateProfileInt("Lambda", "Left", R.left, Filename);
  WritePrivateProfileInt("Lambda", "Top", R.top, Filename);
  WritePrivateProfileInt("Lambda", "Height", R.bottom-R.top, Filename);
  WritePrivateProfileInt("Lambda", "Width", R.right-R.left, Filename);

  WritePrivateProfileBool("Project Explorer", "OpenOnSingleClick", Explorer->SingleClickOpen, Filename);
  WritePrivateProfileBool("Project Explorer", "RenameOnClick", Explorer->RenameOnClick, Filename);
  WritePrivateProfileBool("Project Explorer", "AutomaticlyHide", Explorer->GetAutoHide(), Filename);
  WritePrivateProfileInt("Project Explorer", "AutomaticHideDelay", Explorer->AutoHideDelay/1000, Filename);
  WritePrivateProfileInt("Project Explorer", "Width", SplitterPos, Filename);

  WritePrivateProfileBool("Editor", "OverwriteEmptyDocuments", Editor->OverwriteEmptyDocs, Filename);
  WritePrivateProfileBool("Editor", "OverwriteSelectedText", Editor->OverwriteSelection, Filename);
  WritePrivateProfileBool("Editor", "OverwriteTypingMode", Editor->OverwriteMode, Filename);
  WritePrivateProfileBool("Editor", "IndentAutomaticly", Editor->AutoIndent, Filename);
  WritePrivateProfileBool("Editor", "GroupUndos", Editor->GroupUndo, Filename);
  WritePrivateProfileBool("Editor", "SelectLineOnDoubleClick", Editor->DoubleClickLine, Filename);
  WritePrivateProfileBool("Editor", "SpaceSeparatedWords", Editor->SpaceSeparatedWords, Filename);
  WritePrivateProfileBool("Editor", "TabIndentsLines", Editor->TabIndentsLines, Filename);
  WritePrivateProfileInt("Editor", "TabSize", Editor->TabSize, Filename);
  WritePrivateProfileBool("Editor", "UseDefaultScrollLineCount", Editor->DefaultScrollLineCount, Filename);
  WritePrivateProfileInt("Editor", "ScrollLineCount", Editor->ScrollLineCount, Filename);
  WritePrivateProfileBool("Editor", "GutterVisible", Editor->GetGutterVisible(), Filename);
  WritePrivateProfileBool("Editor", "MarginVisible", Editor->GetMarginVisible(), Filename);
  WritePrivateProfileBool("Editor", "LineNumbersVisible", Editor->GetLineNumbersVisible(), Filename);
  WritePrivateProfileBool("Editor", "AutomaticlyResizeGutter", Editor->GetAutoSizeGutter(), Filename);
  WritePrivateProfileInt("Editor", "GutterSize", Editor->GetGutterSize(), Filename);
  WritePrivateProfileInt("Editor", "MarginSize", Editor->GetMarginSize(), Filename);
  WritePrivateProfileString("Editor", "EditorFontName", Editor->GetFont().Name, Filename);
  WritePrivateProfileInt("Editor", "EditorFontSize", Editor->GetFont().Size, Filename);
  WritePrivateProfileString("Editor", "GutterFontName", Editor->GetGutterFont().Name, Filename);
  WritePrivateProfileInt("Editor", "GutterFontSize", Editor->GetGutterFont().Size, Filename);
}

void UpdateStatusbar()
{
  if (Editor->GetDocument() != NULL)
  {
    /* Cursor position */
    TPoint Pos = Editor->GetDocument()->GetCaretPos();
    string Str;
    char* Chr = IntToStr(Pos.x+1);
    Str.append(Chr);
    delete[] Chr;
    Str.append(" : ");
    Chr = IntToStr(Pos.y+1);
    Str.append(Chr);
    delete[] Chr;
    SendMessage(hStatusbar, SB_SETTEXT, 0, (LPARAM)Str.c_str());
    /* Modified */
    if (Editor->GetDocument()->IsModified())
      SendMessage(hStatusbar, SB_SETTEXT, 1, (LPARAM)"Modified");
    else
      SendMessage(hStatusbar, SB_SETTEXT, 1, (LPARAM)"");
    /* Insert mode */
    if (Editor->OverwriteMode)
      SendMessage(hStatusbar, SB_SETTEXT, 2, (LPARAM)"Overwrite");
    else
      SendMessage(hStatusbar, SB_SETTEXT, 2, (LPARAM)"Insert");
    /* File name */
    SendMessage(hStatusbar, SB_SETTEXT, 3, (LPARAM)Editor->GetDocument()->GetFileName().c_str());
  }
}

void UpdateToolbar()
{
  if (Editor->GetDocument() != NULL)
  {
    /* Save */
    if (Editor->GetDocument()->IsModified())
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_SAVE, TRUE);
    else
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_SAVE, FALSE);
    /* Close */
    if (Editor->DocumentCount() > 1 || Editor->GetDocument()->GetFileName().size() > 0)
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_CLOSEDOC, TRUE);
    else
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_CLOSEDOC, FALSE);
    /* Undo */
    if (Editor->GetDocument()->CanUndo())
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_UNDO, TRUE);
    else
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_UNDO, FALSE);
    /* Redo */
    if (Editor->GetDocument()->CanRedo())
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_REDO, TRUE);
    else
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_REDO, FALSE);
    /* Copy and cut */
    if (Editor->GetDocument()->IsSelected())
    {
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_CUT, TRUE);
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_COPY, TRUE);
    }
    else
    {
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_CUT, FALSE);
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_EDIT_COPY, FALSE);
    }
    /* Remove from project */
    if (Explorer->GetSelectedFile().size() > 0 || Explorer->GetSelectedFolder().size() > 0)
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_REMOVEFROMPROJECT, TRUE);
    else
      SendMessage(hToolbar, TB_ENABLEBUTTON, ID_FILE_REMOVEFROMPROJECT, FALSE);
  }
}

// WINAPI FUNCTIONS ------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch (Message)
  {
    case WM_ACTIVATE:
    {
      if (LOWORD(wParam) != WA_INACTIVE)
        SetFocus(Editor->GetHandle());
      return 0;
    }
    case WM_ACTIVATEAPP:
    {
      if ((BOOL)wParam)
      {
      FILETIME FileTime;
      FILETIME DocTime;
      for (int i = 0; i < Editor->DocumentCount(); i++)
        if (!Editor->GetDocument(i)->GetFileName().empty())
        {
          const char* FileName = Editor->GetDocument(i)->GetFileName().c_str();
          if (GetFileModificationDate(FileName, &FileTime))
          {
            DocTime = Editor->GetDocument(i)->GetFileTime();
            if (CompareFileTime(&DocTime, &FileTime) != 0 && MessageBox(hWnd, "A file has been changed, do you want to reload it?", "Modified file", MB_YESNO) == IDYES)
              Editor->ReloadDocument(i);
          }
        }
      }
      return 0;
    }
    case WM_CLOSE:
    {
      while (Editor->DocumentCount() > 0)
        if (!Editor->CloseDocument(0))
          return 0;
      SaveSettings();
      break;                // The default wndproc must be called.
    }
    case WM_COMMAND:        /* Process control messages */
    {
      if (HIWORD(wParam) == 0 || HIWORD(wParam) == 1)
        MenuProc((short)LOWORD(wParam));
      return 0;
    }
    case WM_CONTEXTMENU:
    {
      HMENU Menu = NULL;
      if ((HWND)wParam == Editor->GetHandle())
        Menu = EditorMenu();
      else if ((HWND)wParam == Explorer->GetHandle())
        Menu = ExplorerMenu();
      else if ((HWND)wParam == hTabs)
        Menu = TabsMenu();
      if (Menu != NULL)
      {
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        if (x < 0 || y < 0)
        {
          RECT R;
          GetWindowRect((HWND)wParam, &R);
          x = R.left+2;
          y = R.top+2;
        }
        TrackPopupMenu(Menu, TPM_LEFTALIGN, x, y, 0, hWnd, NULL);
      }
      return 0;
    }
    case WM_COPYDATA:
    {
      PCOPYDATASTRUCT Data = (PCOPYDATASTRUCT)lParam;
      if (Data->dwData == 1)
      {
        /* Received command line arguments */
        char* Str = new char[MAX_PATH];
        strncpy(Str, (char*)Data->lpData, Data->cbData);
        Str[Data->cbData] = '\0';
        OpenCmdLineDocuments(Str);
        delete[] Str;
      }
      return 0;
    }
    case WM_DESTROY:
    {
      delete Editor;
      delete Explorer;
      CloseHandle(hMutex);
      PostQuitMessage(0);
      return 0;
    }
    case WM_DROPFILES:
    {
      int Count = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, 0, 0);
      for (int i = 0; i < Count; i++)
      {
        char FileName[MAX_PATH];
        DragQueryFile((HDROP)wParam, i, FileName, MAX_PATH);
        if (IsDirectory(FileName))
          OpenDirectory(FileName);
        else
          OpenFile(FileName);
      }
      DragFinish((HDROP)wParam);
      return 0;
    }
    case WM_FILEOPENED:
    {
      if (Editor->DocumentIndex((const char*)wParam) >= 0)
        return 1;
      return 0;
    }
    case WM_MENUSELECT:
    {
      /* Updates the menu items before displaying the menu */
      if ((HMENU)lParam == MainMenu())
      {
        if (GetSubMenu((HMENU)lParam, (UINT)LOWORD(wParam)) == FileMenu())
        {
          /* Save */
          if (Editor->GetDocument()->IsModified())
            EnableMenuItem(MainMenu(), ID_FILE_SAVE, MF_ENABLED);
          else
            EnableMenuItem(MainMenu(), ID_FILE_SAVE, MF_GRAYED);
          /* Close All and Close Others */
          if (Editor->DocumentCount() > 1)
          {
            EnableMenuItem(MainMenu(), ID_FILE_CLOSEOTHERS, MF_ENABLED);
            EnableMenuItem(MainMenu(), ID_FILE_CLOSEALL, MF_ENABLED);
          }
          else
          {
            EnableMenuItem(MainMenu(), ID_FILE_CLOSEOTHERS, MF_GRAYED);
            EnableMenuItem(MainMenu(), ID_FILE_CLOSEALL, MF_GRAYED);
          }
        }
        else if (GetSubMenu((HMENU)lParam, (UINT)LOWORD(wParam)) == EditMenu())
        {
          /* Undo */
          if (Editor->GetDocument()->CanUndo())
            EnableMenuItem(MainMenu(), ID_EDIT_UNDO, MF_ENABLED);
          else
            EnableMenuItem(MainMenu(), ID_EDIT_UNDO, MF_GRAYED);
          /* Redo */
          if (Editor->GetDocument()->CanRedo())
            EnableMenuItem(MainMenu(), ID_EDIT_REDO, MF_ENABLED);
          else
            EnableMenuItem(MainMenu(), ID_EDIT_REDO, MF_GRAYED);
          /* Copy & Cut */
          if (Editor->GetDocument()->IsSelected())
          {
            EnableMenuItem(MainMenu(), ID_EDIT_CUT, MF_ENABLED);
            EnableMenuItem(MainMenu(), ID_EDIT_COPY, MF_ENABLED);
          }
          else
          {
            EnableMenuItem(MainMenu(), ID_EDIT_CUT, MF_GRAYED);
            EnableMenuItem(MainMenu(), ID_EDIT_COPY, MF_GRAYED);
          }
        }
        else if (GetSubMenu((HMENU)lParam, (UINT)LOWORD(wParam)) == ToolMenu())
        {
          /* Automatic convertion */
          CheckMenuItem(MainMenu(), ID_TOOL_AUTOCONVERT, AutoConvertChars ? MF_CHECKED : MF_UNCHECKED);
          EnableMenuItem(MainMenu(), ID_HELP_UPDATE, MF_GRAYED);
        }
      }
      else if ((HMENU)lParam == FileMenu())
      {
        if (GetSubMenu((HMENU)lParam, (UINT)LOWORD(wParam)) == FileNewMenu())
        {
          /* Update the list of templates */
          UpdateMenuTemplates();
        }
      }
      return 0;
    }
    case WM_SELECTDOCUMENT:
    {
      Editor->SetDocument(wParam);
      return 0;
    }
    case WM_SIZE:
    {
      AdjustSize();
      SendMessage(hStatusbar, Message, wParam, lParam);
      return 0;
    }
  }
  return DefWindowProc(hWnd, Message, wParam, lParam);
}

void CreateWindowsApp()
{
  InitCommonControls();
  /* Creates the main window */
  hWindow = CreateWindowEx(WS_EX_ACCEPTFILES, ClassName, WindowTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
    CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, HWND_DESKTOP, NULL, hInstance, NULL);
  /* Creates the menu */
  SetMenu(hWindow, MainMenu());
  /* Creates the toolbar */
  hToolbar = CreateToolbar(hWindow);
  /* Creates the explorer */
  Explorer = new ProjectExplorer(hWindow);
  /* Creates the splitter */
  hSplitter = CreateSplitter(hWindow);
  /* Creates the tabs */
  hTabs = CreateTabs(hWindow);
  DocumentMovedProc = &DocumentMoved;
  /* Creates the editor */
  Editor = new LambdaEditor(hWindow);
  Editor->ActiveDocumentChangedProc = &ActiveDocumentChanged;
  Editor->CursorPosChangedProc = &CursorPosChanged;
  Editor->DocumentAddedProc = &DocumentOpened;
  Editor->DocumentClosedProc = &DocumentClosed;
  Editor->DocumentRenamedProc = &DocumentRenamed;
  Editor->StateChangedProc = &StateChanged;
  Editor->InsertStringProc = &StringInserted;
  /* Creates the status bar */
  int Parts[4] = {100, 200, 300, -1};
  hStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, "", hWindow, ID_STATUSBAR);
  SendMessage(hStatusbar, SB_SETPARTS, (WPARAM)4, (LPARAM)&Parts);
}

bool RegisterWindowsApp()
{
  WNDCLASSEX WndClass;
  /* Specify the window class information */
  WndClass.cbSize = sizeof(WNDCLASSEX);
  WndClass.lpszClassName = ClassName;
  WndClass.hInstance = hInstance;
  WndClass.lpfnWndProc = WindowProc;
  WndClass.style = 0;
  WndClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
  WndClass.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ID_MAINICON), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);
  WndClass.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ID_MAINICON), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION);
  WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  WndClass.lpszMenuName = NULL;
  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  /* Register the new window class */
  return (RegisterClassEx(&WndClass) != 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int Show)
{
  MSG Messages;
  hInstance = hInst;
  /* Checks if a mutex with the class name already exists */
  hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ClassName);
  if (hMutex == NULL)
    hMutex = CreateMutex(NULL, TRUE, ClassName);
  else if (!AllowMultipleInstance)
  {
    HWND PrevWnd = FindPreviousInstance(ClassName);
    if (PrevWnd != NULL)
    {
      SetForegroundWindow(PrevWnd);
      /* Send command line arguments to already running instance */
      if (strlen(cmdLine) != 0)
      {
        COPYDATASTRUCT Data;
        Data.dwData = 1;
        Data.cbData = strlen(cmdLine);
        Data.lpData = cmdLine;
        SendMessage(PrevWnd, WM_COPYDATA, 0, (LPARAM)&Data);
      }
      /* Terminate application */
      return 0;
    }
  }
  /* Register and create the window */
  if (!RegisterWindowsApp())
    return 0;
  /* Create application window and controls */
  CreateWindowsApp();
  LoadSettings();
  Explorer->SetProjectName("New Project");
  Editor->NewDocument();
  ShowWindow(hWindow, Show);
  /* Opens documents */
  OpenCmdLineDocuments(cmdLine);
  /* Message loop */
  HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(ID_MAINACCELERATORS));
  while (GetMessage(&Messages, NULL, 0, 0) != 0)
  {
    /* Limit the accelerators to the main window */
    if (GetActiveWindow() != hWindow || TranslateAccelerator(hWindow, hAccel, &Messages) == FALSE)
      if (IsDialogMessage(hWindow, &Messages) == 0)
      {
        TranslateMessage(&Messages);
        DispatchMessage(&Messages);
      }
  }
  return 0;
}
