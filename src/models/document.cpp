#include <string.h>
#include <fstream>
#include "document.h"
#include "../main.h"

// PUBLIC FUNCTIONS ------------------------------------------------------------

TDocument::TDocument(TObserver* Observer)
{
  AddObserver(Observer);
  FileName = "";
  ZeroMemory(&FileTime, sizeof(FileTime));

  FirstLine = new TStringNode;
  FirstLine->Str = "";
  FirstLine->Prev = NULL;
  FirstLine->Next = NULL;
  CurrentLine = FirstLine;
  LineCount = 1;
  UpdateCount = 0;

  for (int i = 0; i < 10; i++)
    Bookmarks[i] = -1;
  CaretPos.x = 0;
  CaretPos.y = 0;
  LeftCol = 0;
  Modifications = 0;
  SelStart.x = 0;
  SelStart.y = 0;
  SelEnd.x = 0;
  SelEnd.y = 0;
  TopRow = 0;

  NoUndo = false;
  FirstRedo = NULL;
  RedoCount = 0;
  FirstUndo = NULL;
  UndoCount = 0;
}

TDocument::~TDocument()
{
  ClearLines();
  ClearRedo();
  ClearUndo();
}

void TDocument::BeginUpdate()
{
  UpdateCount ++;
}

bool TDocument::CanRedo()
{
  return (FirstRedo != NULL);
}

bool TDocument::CanUndo()
{
  return (FirstUndo != NULL);
}

void TDocument::Clear()
{
  UpdateCount++;
  for (int i = 0; i < 10; i++)
    Bookmarks[i] = -1;
  CaretPos.x = 0;
  CaretPos.y = 0;
  SelStart.x = 0;
  SelStart.y = 0;
  SelEnd.x = 0;
  SelEnd.y = 0;
  LeftCol = 0;
  Modifications = 0;
  TopRow = 0;
  ClearLines();
  ClearRedo();
  ClearUndo();
  UpdateCount--;
  if (UpdateCount == 0)
  {
    Update(UPDATE_CARET);
    Update(UPDATE_VIEW);
  }
  Update(UPDATE_STATE);
}

void TDocument::DeleteString(TPoint Start, TPoint End)
{
  if (PointCmp(Start, End) != 0)
  {
    PStringNode Ptr = GetNode(Start.y, false);
    if (Ptr != NULL)
    {
      AddUndo(Delete, Start, GetString(Start, End));
      /* Delete inside a line */
      if (Start.y == End.y)
      {
        Ptr->Str.erase(Start.x, End.x-Start.x);
        Modifications ++;
        if (UpdateCount == 0)
          Update(UPDATE_LINES, Start.y, Start.y);
      }
      /* Delete entire lines */
      else if (Start.y < End.y)
      {
        UpdateCount++;
        Ptr->Str.erase(Start.x);
        Ptr->Str.append(GetNode(End.y, false)->Str.substr(End.x));
        for (int i = End.y; i > Start.y ; i--)
          DeleteLine(i);
        UpdateCount --;
        Modifications ++;
        if (UpdateCount == 0)
          Update(UPDATE_LINES, Start.y, LineCount+(End.y-Start.y)+1);
      }
      SetCaretPos(Start.x, Start.y);
      Update(UPDATE_STATE);
    }
  }
}

void TDocument::EndUpdate()
{
  UpdateCount--;
  if (UpdateCount == 0)
  {
    Update(UPDATE_CARET);
    Update(UPDATE_VIEW);
  }
}

int TDocument::FindBookmark(int Line)
{
  for (int i = 0; i < 10; i++)
    if (Bookmarks[i] == Line)
      return i;
  return -1;
}

TPoint TDocument::FindString(string Str, bool FromCaret, bool Down, bool MatchCase, bool WholeWord)
{
  TPoint Result;
  bool Found = false;
  if (Str.size() > 0)
  {
    Result.x = (FromCaret) ? CaretPos.x : 0;
    Result.y = (FromCaret) ? CaretPos.y : 0;
    for (Result.y; Result.y < LineCount; Result.y++)
    {
      PStringNode Ptr = GetNode(Result.y, false);
      if (Ptr != NULL)
      {
        if (MatchCase)
          Result.x = Ptr->Str.find(Str, Result.x);
        else
          Result.x = stripos(Ptr->Str.c_str(), Str.c_str(), Result.x);
        if (Result.x >= 0)
          if (!WholeWord || ((Result.x <= 0 || IsDelimiter(Ptr->Str[Result.x-1]))
            && (Result.x+Str.size() >= Ptr->Str.size() || IsDelimiter(Ptr->Str[Result.x+Str.size()]))))
          {
            Found = true;
            break;
          }
      }
    }
  }
  if (!Found)
  {
    Result.x = -1;
    Result.y = -1;
  }
  return Result;
}

void TDocument::FormatLines(int First, int Last, int Width)
{
  PStringNode Ptr = GetNode(First, false);
  UpdateCount++;
  string Str;
  while (Ptr != NULL && First <= Last)
  {
    int Size = GetLineSize(First);
    if (Size > Width)
    {
      int Indent = GetLineIndentation(First);
      int Start = GetWordStart(Point(Width, First), true);
      int End = GetWordEnd(Point(Width, First), true);
      if (Start == Indent || End == Width)
        Start = Ptr->Str.find_first_not_of(" ", End);
      if (Start < Size)
      {
        char* Chars = Spaces(Indent);
        /* Adds to undo list */
        Str.assign("\r\n");
        Str.append(Chars);
        AddUndo(Insert, Point(Start, First), Str);
        /* Breaks the line */
        Str.assign(Chars);
        Str.append(Ptr->Str.substr(Start));
        AddLine(First, Str);
        Ptr-> Str = Ptr->Str.substr(0, Start);
        delete[] Chars;
        Last ++;
      }
    }
    First ++;
    Ptr = Ptr->Next;
  }
  SetCaretPos(GetLineSize(LineCount-1), LineCount-1);
  UpdateCount --;
  Modifications ++;
  if (UpdateCount == 0)
  {
    Update(UPDATE_LINES, 0, LineCount);
    Update(UPDATE_CARET);
    Update(UPDATE_STATE);
  }
}

int TDocument::GetBookmark(int Index)
{
  if (Index < 10)
    return Bookmarks[Index];
  return -1;
}

TPoint TDocument::GetCaretPos()
{
  return CaretPos;
}

string TDocument::GetFileName()
{
  return FileName;
}

FILETIME TDocument::GetFileTime()
{
  return FileTime;
}

int TDocument::GetLeftCol()
{
  return LeftCol;
}

const char* TDocument::GetLine(int Line)
{
  PStringNode Ptr = GetNode(Line, false);
  if (Ptr != NULL)
    return Ptr->Str.c_str();
  return "";
}

int TDocument::GetLineCount()
{
  return LineCount;
}

int TDocument::GetLineIndentation(int Line)
{
  PStringNode Ptr = GetNode(Line, false);
  if (Ptr != NULL)
  {
    int Pos = Ptr->Str.find_first_not_of(" ");
    if (Pos >= 0)
      return Pos;
  }
  return 0;
}

int TDocument::GetLineSize(int Line)
{
  PStringNode Ptr = GetNode(Line, false);
  if (Ptr != NULL)
    return Ptr->Str.size();
  return 0;
}

TPoint TDocument::GetSelStart()
{
  return SelStart;
}

TPoint TDocument::GetSelEnd()
{
  return SelEnd;
}

string TDocument::GetString(TPoint Start, TPoint End)
{
  PStringNode Ptr = GetNode(Start.y, false);
  if (Ptr != NULL && PointCmp(Start, End) != 0)
  {
    string Str;
    if (Start.y == End.y)
      Str = Ptr->Str.substr(Start.x, End.x-Start.x);
    else
    {
      Str = Ptr->Str.substr(Start.x);
      while (Ptr != NULL && Start.y+1 < End.y)
      {
        Ptr = Ptr->Next;
        Str.append("\r\n"+Ptr->Str);
        Start.y ++;
      }
      Ptr = Ptr->Next;
      Str.append("\r\n"+Ptr->Str.substr(0, End.x));
    }
    return Str;
  }
  return "";
}

int TDocument::GetTopRow()
{
  return TopRow;
}

int TDocument::GetType()
{
  return Type;
}

int TDocument::GetWordStart(TPoint Pos, bool SpacesOnly)
{
  int Index = Pos.x;
  PStringNode Ptr = GetNode(Pos.y, false);
  for (Index; Index > 0; Index--)
    if (SpacesOnly)
    {
      if (Ptr->Str[Index-1] == ' ')
        break;
    }
    else
      if (IsDelimiter(Ptr->Str[Index-1]))
        break;
  return Index;
}

int TDocument::GetWordEnd(TPoint Pos, bool SpacesOnly)
{
  int Index = Pos.x;
  PStringNode Ptr = GetNode(Pos.y, false);
  for (Index; Index < Ptr->Str.size(); Index++)
    if (SpacesOnly)
    {
      if (Ptr->Str[Index] == ' ')
        break;
    }
    else
      if (IsDelimiter(Ptr->Str[Index]))
        break;
  return Index;
}

void TDocument::IndentLines(int First, int Last, int IndentSize)
{
  char* Str = Spaces(IndentSize);
  PStringNode Ptr = GetNode(First, false);
  UpdateCount++;
  int i = First;
  while (Ptr != NULL && i <= Last)
  {
    AddUndo(Insert, Point(0, i), Str);
    Ptr->Str.insert(0, Str);
    Ptr = Ptr->Next;
    i ++;
  }
  delete[] Str;
  TPoint Start = {SelStart.x+IndentSize, SelStart.y};
  TPoint End = {SelEnd.x+IndentSize, SelEnd.y};
  SetCaretPos(CaretPos.x+IndentSize, CaretPos.y);
  Select(Start, End);
  UpdateCount --;
  Modifications ++;
  if (UpdateCount == 0)
  {
    Update(UPDATE_LINES, First, Last);
    Update(UPDATE_CARET);
    Update(UPDATE_STATE);
  }
}

void TDocument::InsertChar(TPoint Pos, char chr)
{
  string Str(1, chr);
  Update(UPDATE_STRING, (int)&Str);
  AddUndo(Insert, Pos, Str);
  UpdateCount++;
  PStringNode Ptr = GetNode(Pos.y, false);
  if (Ptr != NULL)
    Ptr->Str.insert(Pos.x, Str);
  else
    AddLine(LineCount-1, Str);
  SetCaretPos(Pos.x+Str.size(), Pos.y);
  UpdateCount --;
  Modifications ++;
  if (UpdateCount == 0)
  {
    Update(UPDATE_LINES, Pos.y, Pos.y);
    Update(UPDATE_CARET);
    Update(UPDATE_STATE);
  }
}

void TDocument::InsertString(TPoint Pos, string Str)
{
  PStringNode Ptr = GetNode(Pos.y, false);
  if (Ptr != NULL)
  {
    Update(UPDATE_STRING, (int)&Str);
    AddUndo(Insert, Pos, Str);
    UpdateCount++;
    int OldLineCount = LineCount;
    int TailSize = Ptr->Str.size()-Pos.x;
    AddLine(Pos.y, Ptr->Str.substr(0, Pos.x)+Str+Ptr->Str.substr(Pos.x));
    DeleteLine(Pos.y);
    int Line = Pos.y+(LineCount-OldLineCount);
    int Col = GetLineSize(Line)-TailSize;
    SetCaretPos(Col, Line);
    UpdateCount --;
    Modifications ++;
    if (UpdateCount == 0)
    {
      Update(UPDATE_LINES, Pos.y, LineCount);
      Update(UPDATE_CARET);
      Update(UPDATE_STATE);
    }
  }
}

bool TDocument::IsEmpty()
{
  PStringNode Ptr = FirstLine;
  if (Ptr == NULL || (Ptr->Str.size() == 0 && Ptr->Next == NULL))
     return true;
  return false;
}

bool TDocument::IsModified()
{
  return (Modifications > 0);
}

bool TDocument::IsSelected()
{
  return (SelStart.y != SelEnd.y || SelStart.x != SelEnd.x);
}

bool TDocument::LoadFromFile(const char* FileName)
{
  bool Result = false;
  ifstream File(FileName, ios::in);
  if (File.is_open())
  {
    UpdateCount ++;
    Clear();
    string Str;
    PStringNode LastLine = NULL;
    while (!File.eof() && getline(File, Str) != 0)
    {
      Update(LOADING_STRING, (int)&Str);
      PStringNode Node = new TStringNode;
      Node->Str.assign(Str);
      Node->Prev = LastLine;
      Node->Next = NULL;
      if (FirstLine == NULL)
        FirstLine = Node;
      else
        LastLine->Next = Node;
      LastLine = Node;
      LineCount++;
    }
    File.close();
    CurrentLine = FirstLine;
    SetFileName(FileName);
    GetFileModificationDate(FileName, &FileTime);
    UpdateCount --;
    if (UpdateCount == 0)
    {
      Update(UPDATE_VIEW);
      Update(UPDATE_CARET);
      Update(UPDATE_STATE);
    }
    Result = true;
  }
  return Result;
}

int TDocument::MaxLineSize()
{
  PStringNode Longest = NULL;
  PStringNode Ptr = FirstLine;
  while (Ptr != NULL)
  {
    if (Longest == NULL || Ptr->Str.size() > Longest->Str.size())
      Longest = Ptr;
    Ptr = Ptr->Next;
  }
  if (Longest != NULL)
    return Longest->Str.size();
  return 0;
}

void TDocument::Redo(bool Group)
{
  if (FirstRedo != NULL)
  {
    PUndoNode Node;
    NoUndo = true;
    do
    {
      Node = FirstRedo;
      FirstRedo = FirstRedo->Next;
      RedoCount = RedoCount-1;
      if (Node->Type == Insert)
        InsertString(Node->Pos, Node->Str);
      else if (Node->Type == Delete)
        DeleteString(Node->Pos, IncrementPos(Node->Pos, Node->Str));
      else if (Node->Type == Caret)
        SetCaretPos(Node->Pos.x, Node->Pos.y, false);
      Node->Next = FirstUndo;
      FirstUndo = Node;
      UndoCount = UndoCount+1;
      Modifications ++;
    }
    while (Group && FirstRedo != NULL && FirstRedo->Type == Node->Type && Node->Type != Caret);
    Update(UPDATE_STATE);
    NoUndo = false;
  }
}

void TDocument::ReplaceString(TPoint Pos, int Length, string Str)
{
  PStringNode Ptr = GetNode(Pos.y, false);
  Ptr->Str.replace(Pos.x, Length, Str);
  Modifications ++;
  if (UpdateCount == 0)
  {
    Update(UPDATE_LINES, Pos.y, Pos.y);
    Update(UPDATE_STATE);
  }
}

bool TDocument::SaveToFile(const char* FileName)
{
  bool Result = false;
  ofstream File(FileName, ios::out);
  if (File.is_open())
  {
    PStringNode Ptr = FirstLine;
    while (Ptr != NULL)
    {
      File.write(Ptr->Str.c_str(), Ptr->Str.size());
      if (Ptr->Next != NULL)
        File.write("\n", 1);
      Ptr = Ptr->Next;
    }
    File.close();
    Modifications = 0;
    SetFileName(FileName);
    GetFileModificationDate(FileName, &FileTime);
    Update(UPDATE_STATE);
    Result = true;
  }
  return Result;
}

void TDocument::Select(TPoint Pos, int Length)
{
  TPoint End = Pos;
  while (End.y < LineCount && End.x+Length > GetLineSize(End.y))
  {
    Length -= GetLineSize(End.y);
    End.y++;
  }
  End.x = Min(End.x+Length, GetLineSize(End.y));
  Select(Pos, End);
}

void TDocument::Select(TPoint Start, TPoint End)
{
  UpdateCount ++;
  TPoint OldStart = SelStart;
  TPoint OldEnd = SelEnd;
  SelStart.y = Max(0, Min(Start.y, LineCount-1));
  SelStart.x = Max(0, Min(Start.x, GetLineSize(Start.y)));
  SelEnd.y = Max(0, Min(End.y, LineCount-1));
  SelEnd.x = Max(0, Min(End.x, GetLineSize(End.y)));
  UpdateCount --;
  if (UpdateCount == 0)
    Update(UPDATE_LINES, Min(OldStart.y, SelStart.y), Max(OldEnd.y, SelEnd.y));
}

void TDocument::SetBookmark(int Index, int Line)
{
  if (Index < 10)
  {
    UpdateCount ++;
    int OldLine = Bookmarks[Index];
    if (Bookmarks[Index] == Line)
      Bookmarks[Index] = -1;
    else
      Bookmarks[Index] = Line;
    UpdateCount --;
    Modifications ++;
    if (UpdateCount == 0)
    {
      if (OldLine >= 0)
        Update(UPDATE_LINES, OldLine, OldLine);
      Update(UPDATE_LINES, Line, Line);
      Update(UPDATE_STATE);
    }
  }
}

void TDocument::SetCaretPos(int X, int Y, bool Selection, bool CanUndo)
{
  Y = Max(0, Min(Y, LineCount-1));
  X = Max(0, Min(X, GetLineSize(Y)));
  if (CanUndo)
    AddUndo(Caret, CaretPos, "");
  TPoint NewPos = {X, Y};
  if (Selection)
  {
    /* Changes the selection */
    TPoint Start = SelStart;
    TPoint End = SelEnd;
    UpdateCount ++;
    if (PointCmp(CaretPos, Start) == 0)      // Changing selection start
    {
      if (PointCmp(NewPos, End) > 0)
        Select(End, NewPos);
      else
        Select(NewPos, End);
      Update(UPDATE_LINES, Min(Start.y, NewPos.y), Max(Start.y, NewPos.y));
    }
    else if (PointCmp(CaretPos, End) == 0)   // Changing selection end
    {
      if (PointCmp(NewPos, Start) < 0)
        Select(NewPos, Start);
      else
        Select(Start, NewPos);
      Update(UPDATE_LINES, Min(End.y, NewPos.y), Max(End.y, NewPos.y));
    }
    UpdateCount--;
  }
  else
    Select(NewPos, NewPos);
  CaretPos = NewPos;
  if (UpdateCount == 0)
    Update(UPDATE_CARET);
}

void TDocument::SetFileName(string FileName)
{
  this->FileName.assign(FileName);
  int ExtPos = FileName.find_last_of('.');
  if (ExtPos >= 0)
  {
    const char* Ext = FileName.substr(ExtPos+1).c_str();
    if (stripos(HTMLDocumentExt, Ext) >= 0)
      Type = HTML_DOCUMENT;
    else if (stripos(HTMLStylesheetExt, Ext) >= 0)
      Type = HTML_STYLESHEET;
    else if (stripos(JavaScriptUnitExt, Ext) >= 0)
      Type = JAVASCRIPT_UNIT;
    else if (stripos(PHPDocumentExt, Ext) >= 0)
      Type = PHP_DOCUMENT;
    else
      Type = TEXT_DOCUMENT;
  }
  Update(UPDATE_FILENAME);
}

void TDocument::SetLeftCol(int Val)
{
  LeftCol = Max(0, Min(Val, MaxLineSize()));
  if (UpdateCount == 0)
    Update(UPDATE_VIEW);
}

void TDocument::SetTopRow(int Val)
{
  TopRow = Max(0, Min(Val, LineCount-1));
  CurrentLine = GetNode(TopRow, true);
  if (UpdateCount == 0)
    Update(UPDATE_VIEW);
}

void TDocument::SetType(int Val)
{
  Type = Val;
  if (UpdateCount == 0)
    Update(UPDATE_VIEW);
}

void TDocument::Undo(bool Group)
{
  if (FirstUndo != NULL)
  {
    PUndoNode Node;
    NoUndo = true;
    do
    {
      Node = FirstUndo;
      FirstUndo = FirstUndo->Next;
      UndoCount = UndoCount-1;
      if (Node->Type == Insert)
        DeleteString(Node->Pos, IncrementPos(Node->Pos, Node->Str));
      else if (Node->Type == Delete)
        InsertString(Node->Pos, Node->Str);
      else if (Node->Type == Caret)
        SetCaretPos(Node->Pos.x, Node->Pos.y, false);
      Node->Next = FirstRedo;
      FirstRedo = Node;
      RedoCount = RedoCount+1;
      Modifications --;
    }
    while (Group && FirstUndo != NULL && FirstUndo->Type == Node->Type && Node->Type != Caret);
    Update(UPDATE_STATE);
    NoUndo = false;
  }
}

void TDocument::UnindentLines(int First, int Last, int TabSize)
{
  int Size;
  UpdateCount ++;
  TPoint Start = SelStart;
  TPoint End = SelEnd;
  PStringNode Ptr = GetNode(First, false);
  int i = First;
  while (Ptr != NULL && i <= Last)
  {
    Size = Min(Ptr->Str.find_first_not_of(' ', 0), TabSize);
    if (Size > 0)
    {
      char* Str = Spaces(Size);
      AddUndo(Delete, Point(0, i), Str);
      delete[] Str;
      Ptr->Str.erase(0, Size);
      if (i == CaretPos.y)
        SetCaretPos(CaretPos.x-Size, CaretPos.y);
      if (i == Start.y)
        Start.x -= Size;
      if (i == End.y)
        End.x -= Size;
    }
    Ptr = Ptr->Next;
    i ++;
  }
  Select(Start, End);
  UpdateCount --;
  Modifications ++;
  if (UpdateCount == 0)
  {
    Update(UPDATE_LINES, First, Last);
    Update(UPDATE_CARET);
    Update(UPDATE_STATE);
  }
}

// PRIVATE FUNCTIONS -----------------------------------------------------------

void TDocument::AddUndo(TUndoType Type, TPoint Pos, const string Str)
{
  if (!NoUndo)
  {
    PUndoNode Node = new TUndoNode;
    Node->Type = Type;
    Node->Pos = Pos;
    Node->Str.assign(Str);
    Node->Next = FirstUndo;
    FirstUndo = Node;
    UndoCount = UndoCount+1;
    ClearRedo();
  }
}

void TDocument::AddLine(int Line, string Str)
{
  short LineBreak;
  int Start = 0;
  int End = 0;
  PStringNode Ptr = GetNode(Line, false);
  do
  {
    LineBreak = 1;
    End = Str.find("\r\n", Start);
    if (End >= 0)
      LineBreak = 2;
    else
    {
      End = Str.find_first_of("\r\n", Start);
      if (End < 0)
        End = Str.size();
    }
    PStringNode Node = new TStringNode;
    Node->Str.assign(Str.substr(Start, End-Start));
    Node->Prev = Ptr;
    Node->Next = NULL;
    if (Ptr != NULL)
    {
      Node->Next = Ptr->Next;
      if (Ptr->Next != NULL)
        Ptr->Next->Prev = Node;
      Ptr->Next = Node;
    }
    else
    {
      FirstLine = Node;
      CurrentLine = FirstLine;
    }
    LineCount++;
    for (int i = 0; i < 10; i++)
      if (Bookmarks[i] > (int)Line)
        Bookmarks[i] += 1;
    Ptr = Node;
    Start = End + LineBreak;
  }
  while (Start <= Str.size());
  CurrentLine = GetNode(TopRow, true);
}

void TDocument::ClearLines()
{
  PStringNode Ptr;
  while (FirstLine != NULL)
  {
    Ptr = FirstLine;
    FirstLine = FirstLine->Next;
    delete Ptr;
  }
  FirstLine = NULL;
  CurrentLine = FirstLine;
  LineCount = 0;
}

void TDocument::ClearRedo()
{
  PUndoNode Ptr;
  while (FirstRedo != NULL)
  {
    Ptr = FirstRedo;
    FirstRedo = FirstRedo->Next;
    delete Ptr;
  }
  FirstRedo = NULL;
  RedoCount = 0;
}

void TDocument::ClearUndo()
{
  PUndoNode Ptr;
  while (FirstUndo != NULL)
  {
    Ptr = FirstUndo;
    FirstUndo = FirstUndo->Next;
    delete Ptr;
  }
  FirstUndo = NULL;
  UndoCount = 0;
}

void TDocument::DeleteLine(int Line)
{
  PStringNode Ptr = GetNode(Line, false);
  if (Ptr != NULL)
  {
    if (LineCount > 1)
    {
      if (Ptr->Prev != NULL)
        Ptr->Prev->Next = Ptr->Next;
      else
        FirstLine = Ptr->Next;
      if (Ptr->Next != NULL)
        Ptr->Next->Prev = Ptr->Prev;
      delete Ptr;
      LineCount--;
    }
    else
      Ptr->Str.clear();
    for (int i = 0; i < 10; i++)
      if (Bookmarks[i] > (int)Line)
        Bookmarks[i] -= 1;
    CurrentLine = GetNode(TopRow, true);
  }
}

PStringNode TDocument::GetNode(int Line, bool FromTop)
{
  PStringNode Ptr;
  /* Find line from top */
  if (FromTop)
  {
    Ptr = FirstLine;
    while (Ptr != NULL && Line > 0)
    {
      Ptr = Ptr->Next;
      Line--;
    }
  }
  /* Find line from current line */
  else
  {
    Ptr = CurrentLine;                    // if (Line == TopRow)
    while (Ptr != NULL && Line < TopRow)  // if (Line < TopRow)
    {
      Ptr = Ptr->Prev;
      Line++;
    }
    while (Ptr != NULL && Line > TopRow)  // if (Line > TopRow)
    {
      Ptr = Ptr->Next;
      Line--;
    }
  }
  return Ptr;
}

TPoint TDocument::IncrementPos(const TPoint Pos, const string Str)
{
  TPoint Result = Pos;
  bool LineBreak;
  int Start = 0;
  int End = 0;
  do
  {
    End = Str.find("\r\n", Start);
    if (End >= 0)
      LineBreak = true;
    else
    {
      End = Str.find_first_of("\r\n", Start);
      End = (End < 0 ? Str.size() : End);
      LineBreak = false;
    }
    if (Start > 0)
    {
      Result.y ++;
      Result.x = End-Start;
    }
    else
      Result.x += End-Start;
    Start = End + (LineBreak ? 2 : 1);
  }
  while (Start <= (int)Str.size());
  return Result;
}

bool TDocument::IsDelimiter(char c)
{
  int i = (int)c;
  return ((i >= 32 && i <= 34) || (i >= 39 && i <= 47) || (i >= 58 && i <= 64)
          || (i >= 91 && i <= 96) || (i >= 123 && i <= 127));
}
