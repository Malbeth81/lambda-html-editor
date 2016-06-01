#ifndef DOCUMENT_H
#define	DOCUMENT_H

#include <string>
using namespace std;

#include "observable.h"
#include "../tools.h"

#define TEXT_DOCUMENT 101
#define HTML_DOCUMENT 102
#define HTML_STYLESHEET 103
#define JAVASCRIPT_UNIT 104
#define PHP_DOCUMENT 105

#define HTMLDocumentExt "html;htm;htt;shtml;xht;xhtml;xml"
#define HTMLStylesheetExt "css;xsl"
#define JavaScriptUnitExt "js"
#define PHPDocumentExt "php;php4"

#define LOADING_STRING 9
#define UPDATE_FILENAME 10
#define UPDATE_CARET 11
#define UPDATE_LINES 12
#define UPDATE_STATE 13
#define UPDATE_STRING 14
#define UPDATE_VIEW 15

typedef enum {Insert, Delete, Caret} TUndoType;

typedef struct TUndoNode {
  TUndoType Type;
  TPoint Pos;
  string Str;
  TUndoNode* Next;
} *PUndoNode;

typedef struct TStringNode {
  string Str;
  TStringNode* Prev;
  TStringNode* Next;
} *PStringNode;

typedef class TDocument : public TSubject {
  public:
    TDocument(TObserver* Observer);
    ~TDocument();

    void BeginUpdate();
    bool CanRedo();
    bool CanUndo();
    void Clear();
    void DeleteString(TPoint Start, TPoint End);
    void EndUpdate();
    int FindBookmark(int Line);
    TPoint FindString(string Str, bool FromCaret, bool Down, bool MatchCase, bool WholeWord);
    void FormatLines(int First, int Last, int Width);
    int GetBookmark(int Index);
    TPoint GetCaretPos();
    string GetFileName();
    FILETIME GetFileTime();
    int GetLeftCol();
    const char* GetLine(int Line);
    int GetLineCount();
    int GetLineIndentation(int Line);
    int GetLineSize(int Line);
    TPoint GetSelStart();
    TPoint GetSelEnd();
    string GetString(TPoint Start, TPoint End);
    int GetTopRow();
    int GetType();
    int GetWordStart(TPoint Pos, bool SpacesOnly);
    int GetWordEnd(TPoint Pos, bool SpacesOnly);
    void IndentLines(int First, int Last, int IndentSize);
    void InsertChar(TPoint Pos, char Chr);
    void InsertString(TPoint Pos, string Str);
    bool IsEmpty();
    bool IsModified();
    bool IsSelected();
    bool LoadFromFile(const char* FileName);
    int MaxLineSize();
    void Redo(bool Group = false);
    void ReplaceString(TPoint Pos, int Length, string Str);
    bool SaveToFile(const char* FileName);
    void Select(TPoint Pos, int Length);
    void Select(TPoint Pos, TPoint End);
    void SetBookmark(int Index, int Line);
    void SetCaretPos(int X, int Y, bool Selection = false, bool CanUndo = false);
    void SetFileName(string FileName);
    void SetLeftCol(int Val);
    void SetTopRow(int Val);
    void SetType(int Val);
    void Undo(bool Group = false);
    void UnindentLines(int First, int Last, int TabSize);
  private:
    string FileName;
    FILETIME FileTime;
    int Type;

    PStringNode FirstLine;
    PStringNode CurrentLine;
    int LineCount;
    int UpdateCount;

    int Bookmarks[10];
    TPoint CaretPos;
    int LeftCol;
    int Modifications;
    TPoint SelStart;
    TPoint SelEnd;
    int TopRow;

    bool NoUndo;
    PUndoNode FirstRedo;
    short RedoCount;
    PUndoNode FirstUndo;
    short UndoCount;

    void AddUndo(TUndoType Type, TPoint Pos, const string Str);
    void AddLine(int Line, string Str);
    void ClearLines();
    void ClearRedo();
    void ClearUndo();
    void DeleteLine(int Line);
    PStringNode GetNode(int Line, bool FromTop);
    TPoint IncrementPos(const TPoint Pos, const string Str);
    bool IsDelimiter(char c);
} *PDocument;


#endif
