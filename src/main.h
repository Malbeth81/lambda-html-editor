#ifndef MAIN_H
#define	MAIN_H

#include "controls/editor.h"
#include "controls/explorer.h"

#define DefaultFileExt "txt"
#define ProjectFileExt "lpg"

#define SupportedFilesExtCount 44
#define SingleTagsCount 11
#define PairedTagsCount 70
#define CharactersCount 101

extern HINSTANCE hInstance;
extern HWND hWindow;
extern HWND hStatusbar;
extern LambdaEditor* Editor;
extern ProjectExplorer* Explorer;

extern bool AllowMultipleInstance;
extern bool AutoConvertChars;
extern bool AutoUpdate;
extern bool OpenUnsupportedFiles;
extern bool DefaultInternetBrowser;
extern string InternetBrowser;

extern const char* SingleTags[];
extern const char* PairedTags[];
extern const char* CharCodes[];
extern const char* Characters[];

void AdjustSize();
void AddToProject();
void ChangeHTMLTagCase(bool UpperCase);
bool Find(const char* FindWhat, bool FromCaret, bool Down, bool MatchCase, bool WholeWords);
RECT GetClientRect();
void Open();
void OpenDocument(const char* FileName);
void OpenFile(const char* FileName);
void OpenInBrowser(const char* FileName);
void Quit();
void ReplaceSelection(const char* ReplaceWith);
void ReplaceAll(const char* FindWhat, const char* ReplaceWith, bool FromCaret, bool MatchCase, bool WholeWords);
void SaveAll();
void SaveAsTemplate(int Index);
void SaveDocument(int Index);
void SaveDocumentAs(int Index);
void SaveProject();
void SaveProjectAs();
void SaveSettings();
void UpdateStatusbar();
void UpdateToolbar();

#endif
