#ifndef EDITOR_H
#define	EDITOR_H

#include "../models/documentlist.h"
#include "../tools.h"

typedef void (__stdcall *HACTIVEDOCCHANGEDPROC)();
typedef void (__stdcall *HCURSORPOSCHANGEDPROC)();
typedef void (__stdcall *HDOCADDEDPROC)(const char* FileName);
typedef bool (__stdcall *HDOCCLOSEDPROC)(int Index);
typedef void (__stdcall *HDOCRENAMEDPROC)(int Index, const char* FileName);
typedef void (__stdcall *HINSERTSTRINGPROC)(string* Str);
typedef void (__stdcall *HMODECHANGEDPROC)();
typedef void (__stdcall *HSTATECHANGEDPROC)();

struct LambdaEditorColors {
    COLORREF Window;
    COLORREF WindowText;
    COLORREF Gutter;
    COLORREF GutterText;
    COLORREF Bookmark;
    COLORREF BookmarkText;
    COLORREF Highlight;
    COLORREF HighlightText;
    COLORREF Margin;
};

class LambdaEditor : public TObserver {
  public :
    bool OverwriteEmptyDocs;
    bool OverwriteSelection;
    bool OverwriteMode;
    bool AutoIndent;
    bool GroupUndo;
    bool DoubleClickLine;
    bool SpaceSeparatedWords;
    bool TabIndentsLines;
    int TabSize;
    bool DefaultScrollLineCount;
    int ScrollLineCount;

    HACTIVEDOCCHANGEDPROC ActiveDocumentChangedProc;
    HCURSORPOSCHANGEDPROC CursorPosChangedProc;
    HDOCADDEDPROC DocumentAddedProc;
    HDOCCLOSEDPROC DocumentClosedProc;
    HDOCRENAMEDPROC DocumentRenamedProc;
    HMODECHANGEDPROC ModeChangedProc;
    HINSERTSTRINGPROC InsertStringProc;
    HSTATECHANGEDPROC StateChangedProc;

    LambdaEditor(HWND hParent);
    ~LambdaEditor();
    void BeginUpdate();
    bool CloseDocument(int Index);
    void CopyToClipboard();
    void CutToClipboard();
    int DocumentCount();
    int DocumentIndex();
    int DocumentIndex(string FileName);
    void EndUpdate();
    HWND GetHandle();
    PDocument GetDocument();
    PDocument GetDocument(int Index);
    void GotoBookmark(int Index);
    void MoveDocument(int Index, int NewIndex);
    void NewDocument();
    void LoadDocument(const char* FileName);
    void PasteFromClipboard();
    void Print(HDC DC);
    void ReloadDocument(int Index);
    bool SaveDocument(int Index);
    void SelectAll();
    void SetDocument(int Index);
    void ToggleBookmark(int Index);

    bool GetAutoSizeGutter();
    LambdaEditorColors GetColors();
    TFont GetFont();
    TFont GetGutterFont();
    int GetGutterSize();
    bool GetGutterVisible();
    bool GetLineNumbersVisible();
    int GetMarginSize();
    bool GetMarginVisible();

    void SetAutoSizeGutter(bool);
    void SetColors(LambdaEditorColors);
    void SetFont(TFont);
    void SetGutterFont(TFont);
    void SetGutterSize(int);
    void SetGutterVisible(bool);
    void SetLineNumbersVisible(bool);
    void SetMarginSize(int);
    void SetMarginVisible(bool);

  private :
    HWND Handle;
    HCURSOR ArrowCursor;
    HCURSOR BeamCursor;

    int CaretHeight;
    int CaretWidth;
    bool CaretVisible;
    int CellHeight;
    int CellWidth;
    PDocument Document;
    TDocumentList DocumentList;
    int GutterCellHeight;
    int GutterCellWidth;
    int GutterWidth;
    int Height;
    int UpdateCount;
    int VisibleRows;
    int VisibleCols;
    int Width;

    bool AutoSizeGutter;
    LambdaEditorColors Colors;
    TFont Font;
    TFont GutterFont;
    int GutterSize;
    bool GutterVisible;
    bool LineNumbersVisible;
    int MarginSize;
    bool MarginVisible;

    /* GUI functions */
    void AdjustGutterSize();
    void AdjustView();
    int CalculateIndentation(int Pos);
    int IndentSize(int Pos);
    void Invalidate();
    TPoint MouseToCaret(const int x, const int y);
    void PaintCaret();
    void PaintGutter(HDC DC, int First, int Last);
    void PaintLine(HDC DC, int Line, int First, int Last);
    void ReplaceTabs(string* Str);
    void SetHScrollBar(int Pos, int Range);
    void SetVScrollBar(int Pos, int Range);
    void Update(int Message, int wParam, int lParam);
    void UpdateCellSize();
    void UpdateScrollbars();
    void UpdateSize(int NewWidth, int NewHeight);
    /* Event functions */
    int Char(int c);
    void DoubleClick(int Keys, int x, int y);
    void HScroll(int ScrollCode, int Pos);
    int KeyDown(int Key);
    void MouseWheel(int Keys, int Delta, int x, int y);
    void Paint();
    void VScroll(int ScrollCode, int Pos);
    /* WinAPI functions */
    HWND CreateWnd(HWND hParent);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
