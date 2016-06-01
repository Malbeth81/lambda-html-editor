#define _WIN32_WINDOWS 0x0500
#include <windows.h>
#include <math.h>
#include "editor.h"

const char ClassName[] = "LambdaEditor";

// PUBLIC FUNCTIONS ------------------------------------------------------------

LambdaEditor::LambdaEditor(HWND hParent)
{
  Handle = CreateWnd(hParent);
  if (Handle != NULL)
  {
    OverwriteEmptyDocs = true;
    OverwriteSelection = true;
    OverwriteMode = false;
    AutoIndent = true;
    GroupUndo = false;
    DoubleClickLine = false;
    SpaceSeparatedWords = false;
    TabIndentsLines = false;
    TabSize = 2;
    DefaultScrollLineCount = true;
    ScrollLineCount = 3;

    ActiveDocumentChangedProc = NULL;
    CursorPosChangedProc = NULL;
    DocumentAddedProc = NULL;
    DocumentClosedProc = NULL;
    StateChangedProc = NULL;
    InsertStringProc = NULL;

    ArrowCursor = LoadCursor(NULL, IDC_ARROW);
    BeamCursor = LoadCursor(NULL, IDC_IBEAM);

    CaretHeight = 16;
    CaretWidth = 1;
    CaretVisible = false;
    CellHeight = 16;
    CellWidth = 8;
    Document = NULL;
    GutterCellHeight = 16;
    GutterCellWidth = 8;
    GutterWidth = 22;
    Height = 300;
    UpdateCount = 0;
    VisibleRows = 0;
    VisibleCols = 0;
    Width = 500;

    AutoSizeGutter = true;
    Colors.Window = GetSysColor(COLOR_WINDOW);
    Colors.WindowText = GetSysColor(COLOR_WINDOWTEXT);
    Colors.Gutter = GetSysColor(COLOR_BTNFACE);
    Colors.GutterText = GetSysColor(COLOR_GRAYTEXT);
    Colors.Bookmark = GetSysColor(COLOR_BTNHIGHLIGHT);
    Colors.BookmarkText = GetSysColor(COLOR_BTNTEXT);
    Colors.Highlight = GetSysColor(COLOR_HIGHLIGHT);
    Colors.HighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    Colors.Margin = GetSysColor(COLOR_BTNFACE);
    strcpy(Font.Name, "Courier New");
    Font.Size = 10;
    Font.Style = 0;
    GutterFont = Font;
    GutterSize = 22;
    GutterVisible = true;
    LineNumbersVisible = true;
    MarginSize = 80;
    MarginVisible = true;
  }
}

LambdaEditor::~LambdaEditor()
{
}

void LambdaEditor::BeginUpdate()
{
  UpdateCount ++;
}
    
bool LambdaEditor::CloseDocument(int Index)
{
  bool Result = true;
  if (DocumentClosedProc != NULL)
    Result = (*DocumentClosedProc)(Index);
  if (Result)
  {
    DocumentList.Remove(Index);
    SetDocument(Index);
  }
  return Result;
}

void LambdaEditor::CopyToClipboard()
{
  if (Document != NULL && Document->IsSelected())
  {
    if (OpenClipboard(Handle))
    {
      EmptyClipboard();
      string Sel = Document->GetString(Document->GetSelStart(), Document->GetSelEnd());
      HGLOBAL Mem = GlobalAlloc(GMEM_MOVEABLE, (Sel.size()+1)*sizeof(char));
      if (Mem != NULL)
      {
        char* Str = (char*)GlobalLock(Mem);
        strncpy(Str, Sel.c_str(), Sel.size());
        Str[Sel.size()] = '\0';
        GlobalUnlock(Mem);
        SetClipboardData(CF_TEXT, Mem);
      }
      CloseClipboard();
    }
  }
}

void LambdaEditor::CutToClipboard()
{
  if (Document != NULL && Document->IsSelected())
  {
    CopyToClipboard();
    Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
  }
}

int LambdaEditor::DocumentCount()
{
  return DocumentList.Count();
}

int LambdaEditor::DocumentIndex()
{
  for (int i = 0; i < DocumentList.Count(); i++)
    if (DocumentList[i] == Document)
      return i;
  return -1;
}

int LambdaEditor::DocumentIndex(string FileName)
{
  for (int i = 0; i < DocumentList.Count(); i++)
    if (DocumentList[i]->GetFileName() == FileName)
      return i;
  return -1;
}

void LambdaEditor::EndUpdate()
{
  UpdateCount--;
  if (UpdateCount == 0)
    Invalidate();
}

HWND LambdaEditor::GetHandle()
{
  return Handle;
}

PDocument LambdaEditor::GetDocument()
{
  return Document;
}

PDocument LambdaEditor::GetDocument(int Index)
{
  return DocumentList[Index];
}

void LambdaEditor::GotoBookmark(int Index)
{
  int Line = Document->GetBookmark(Index);
  if (Line >= 0)
    Document->SetCaretPos(0, Line, true);
}

void LambdaEditor::MoveDocument(int Index, int NewIndex)
{
  DocumentList.Move(Index, NewIndex);
  SetDocument(NewIndex);
}
    
void LambdaEditor::NewDocument()
{
  PDocument Document = new TDocument(this);
  DocumentList.Add(Document);
  if (DocumentAddedProc != NULL)
    (*DocumentAddedProc)("");
  SetDocument(DocumentList.Count()-1);
}

void LambdaEditor::LoadDocument(const char* FileName)
{
  /* Select already opened document */
  for (int i = 0; i < DocumentList.Count(); i++)
    if (DocumentList[i]->GetFileName().compare(FileName) == 0)
    {
      SetDocument(DocumentIndex(FileName));
      return;
    }
  /* Opens the document */
  if (Document == NULL || !Document->IsEmpty() || !OverwriteEmptyDocs)
    NewDocument();
  if (Document->LoadFromFile(FileName))
    if (DocumentRenamedProc != NULL)
      (*DocumentRenamedProc)(DocumentIndex(), FileName);
}

void LambdaEditor::PasteFromClipboard()
{
  if (Document != NULL && IsClipboardFormatAvailable(CF_TEXT))
    if (OpenClipboard(Handle))
    {
      if (OverwriteSelection)
        Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
      HGLOBAL Mem = GetClipboardData(CF_TEXT);
      if (Mem != NULL)
      {
        string Str((char*)GlobalLock(Mem));
        Document->InsertString(Document->GetCaretPos(), Str);
        GlobalUnlock(Mem);
      }
      CloseClipboard();
    }
}

void LambdaEditor::Print(HDC DC)
{
  if (DC != NULL)
  {
    SIZE Size;
    HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &Font));
    GetTextExtentPoint32(DC, "W", 1, &Size);
    int CharHeight = Max(0, Size.cy);
    int CharWidth = Max(0, Size.cx);
    int PageWidth = GetDeviceCaps(DC, HORZRES)-GetDeviceCaps(DC, PHYSICALOFFSETX);
    int PageHeight = GetDeviceCaps(DC, VERTRES)-GetDeviceCaps(DC, PHYSICALOFFSETY);
    int CharPerLines = Max(1, (int)floor((float)PageWidth/CharWidth));
    int LinesPerPage = Max(1, (int)floor((float)PageHeight/CharHeight));

    DOCINFO DocInfo;
    DocInfo.cbSize = sizeof(DocInfo);
    DocInfo.lpszDocName = "Lambda Document";
    DocInfo.lpszOutput = 0;
    DocInfo.lpszDatatype = "";
    DocInfo.fwType = 0;

    if (StartDoc(DC, &DocInfo) > 0)
    {
      StartPage(DC);
      int LineCount = 0;
      for (int i = 0; i < Document->GetLineCount(); i++)
      {
        const char* Str = Document->GetLine(i);
        if (LineCount > LinesPerPage)
        {
          EndPage(DC);
          StartPage(DC);
          LineCount = 0;
        }
        for (int j = 0; j < strlen(Str); j++)
        {
          int Size = Min(strlen(Str)-j, CharPerLines);
          TextOut(DC, 0, LineCount*CharHeight, Str+j, Size);
          LineCount ++;
          j += Size-1;
        }
      }
      EndPage(DC);
      EndDoc(DC);
    }
    DeleteObject(SelectObject(DC, OldFont));
  }
}

void LambdaEditor::ReloadDocument(int Index)
{
  DocumentList[Index]->LoadFromFile(DocumentList[Index]->GetFileName().c_str());
}

bool LambdaEditor::SaveDocument(int Index)
{
  if (DocumentList[Index] != NULL)
    return DocumentList[Index]->SaveToFile(DocumentList[Index]->GetFileName().c_str());
  return false;
}

void LambdaEditor::SelectAll()
{
  if (Document != NULL)
  {
    int LineCount = Document->GetLineCount();
    Document->SetCaretPos(Document->GetLineSize(LineCount-1), LineCount-1, true);
    Document->Select(Point(0, 0), Point(Document->GetLineSize(LineCount-1), LineCount-1));
  }
}

void LambdaEditor::SetDocument(int Index)
{
  Index = Min(DocumentList.Count()-1, Index);
  Document = DocumentList[Index]; // Returns NULL when index out of bound
  Invalidate();
  PaintCaret();
  UpdateScrollbars();
  if (ActiveDocumentChangedProc != NULL)
    (*ActiveDocumentChangedProc)();
}

void LambdaEditor::ToggleBookmark(int Index)
{
  if (Document != NULL)
    Document->SetBookmark(Index, Document->GetCaretPos().y);
}

// PUBLIC ACCESS FUNCTIONS -----------------------------------------------------

bool LambdaEditor::GetAutoSizeGutter()
{
  return AutoSizeGutter;
}

LambdaEditorColors LambdaEditor::GetColors()
{
  return Colors;
}

TFont LambdaEditor::GetFont()
{
  return Font;
}

TFont LambdaEditor::GetGutterFont()
{
  return GutterFont;
}

int LambdaEditor::GetGutterSize()
{
  return GutterSize;
}

bool LambdaEditor::GetGutterVisible()
{
  return GutterVisible;
}

bool LambdaEditor::GetLineNumbersVisible()
{
  return LineNumbersVisible;
}

int LambdaEditor::GetMarginSize()
{
  return MarginSize;
}

bool LambdaEditor::GetMarginVisible()
{
  return MarginVisible;
}

// PUBLIC MUTATOR FUNCTIONS ----------------------------------------------------

void LambdaEditor::SetAutoSizeGutter(bool Value)
{
  AutoSizeGutter = Value;
  Invalidate();
}

void LambdaEditor::SetColors(LambdaEditorColors Colors)
{
  this->Colors = Colors;
  Invalidate();
}

void LambdaEditor::SetFont(TFont Font)
{
  this->Font = Font;
  UpdateCellSize();
  Invalidate();
}

void LambdaEditor::SetGutterFont(TFont Font)
{
  GutterFont = Font;
  UpdateCellSize();
  Invalidate();
}

void LambdaEditor::SetGutterSize(int Value)
{
  GutterSize = Value;
  Invalidate();
}

void LambdaEditor::SetGutterVisible(bool Value)
{
  GutterVisible = Value;
  Invalidate();
}

void LambdaEditor::SetLineNumbersVisible(bool Value)
{
  LineNumbersVisible = Value;
  Invalidate();
}

void LambdaEditor::SetMarginSize(int Value)
{
  MarginSize = Value;
  Invalidate();
}

void LambdaEditor::SetMarginVisible(bool Value)
{
  MarginVisible = Value;
  Invalidate();
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

void LambdaEditor::AdjustGutterSize()
{
  int OldSize = GutterWidth;
  /* Calculate gutter size */
  if (!GutterVisible)
    GutterWidth = 0;
  else if (AutoSizeGutter && LineNumbersVisible)
  {
    char* Str = IntToStr(Max(VisibleRows, Document->GetLineCount()));
    GutterWidth = Max(0, GutterCellWidth*strlen(Str))+6;
    delete[] Str;
  }
  else
    GutterWidth = GutterSize;
  /* Move the caret if size has changed */
  if (GutterWidth != OldSize)
    PaintCaret();
}

void LambdaEditor::AdjustView()
{
  if (Document != NULL)
  {
    TPoint Caret = Document->GetCaretPos();
    /* Horizontal view position */
    if (Caret.x < Document->GetLeftCol())
      Document->SetLeftCol(Max(0, Caret.x-5));
    else if (Caret.x >= Document->GetLeftCol()+VisibleCols)
      Document->SetLeftCol(Caret.x-VisibleCols);
    /* Vertical view position */
    if (Caret.y < Document->GetTopRow())
      Document->SetTopRow(Max(0, Caret.y-1));
    else if (Caret.y >= Document->GetTopRow()+VisibleRows)
      Document->SetTopRow(Caret.y-VisibleRows);
  }
}

int LambdaEditor::IndentSize(int Pos)
{
  return TabSize-(Pos%TabSize);
}

void LambdaEditor::Invalidate()
{
  if (UpdateCount == 0)
    InvalidateRect(Handle, NULL, FALSE);
}

TPoint LambdaEditor::MouseToCaret(const int x, const int y)
{
  TPoint Result = {0, 0};
  if (Document != NULL)
  {
    Result.y = Min(Max(0, (y-1)/CellHeight+Document->GetTopRow()), Document->GetLineCount()-1);
    Result.x = Min(Max(0, (x-GutterWidth-1+CellWidth/2)/CellWidth+Document->GetLeftCol()), Document->GetLineSize(Result.y));
  }
  return Result;
}

void LambdaEditor::PaintCaret()
{
  if (UpdateCount == 0)
  {
    /* If the caret is inside the visible part of the document */
    if (Document != NULL && Document->GetCaretPos().x >= Document->GetLeftCol() && Document->GetCaretPos().y >= Document->GetTopRow()
    && Document->GetCaretPos().x <= Document->GetLeftCol()+VisibleCols && Document->GetCaretPos().y <= Document->GetTopRow()+VisibleRows)
    {
      /* Show the caret */
      SetCaretPos(GutterWidth+1+(Document->GetCaretPos().x-Document->GetLeftCol())*CellWidth, 1+(Document->GetCaretPos().y-Document->GetTopRow())*CellHeight);
      if (!CaretVisible)
        if (ShowCaret(Handle))
          CaretVisible = true;
    }
    else if (CaretVisible)
    {
      /* Hide the caret */
      HideCaret(Handle);
      CaretVisible = false;
    }
  }
}

void LambdaEditor::PaintGutter(HDC DC, int First, int Last)
{
  if (Document != NULL)
  {
    int X = 0;
    int Y = (First-Document->GetTopRow())*CellHeight+1;
    HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &GutterFont));
    for (int i = First; i <= Last; i++)
    {
      SetBkMode(DC, OPAQUE);
      /* Paints the background */
      HBRUSH OldBrush = (HBRUSH)SelectObject(DC, CreateSolidBrush(Colors.Gutter));
      HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Gutter));
      Rectangle(DC, X, Y, X+GutterWidth-2, Y+CellHeight);
      /* Paints the inner border */
      DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT))));
      MoveToEx(DC, GutterWidth-2, Y, NULL);
      LineTo(DC, GutterWidth-2, Y+CellHeight);
      /* Paints the outer border */
      DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW))));
      MoveToEx(DC, GutterWidth-1, Y, NULL);
      LineTo(DC, GutterWidth-1, Y+CellHeight);
      /* Paints the line numbers and bookmarks */
      SetBkMode(DC, TRANSPARENT);
      int y = Y+(CellHeight-GutterCellHeight)/2;
      if (Document->FindBookmark(i) >= 0)
      {
        int x = (GutterWidth-GutterCellWidth-6)/2;
        DeleteObject(SelectObject(DC, CreateSolidBrush(Colors.Bookmark)));
        DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW))));
        RoundRect(DC, x, y+1, x+GutterCellWidth+4, y+GutterCellHeight-1, 4, 4);
        SetTextColor(DC, Colors.BookmarkText);
        char* Str = IntToStr((Document->FindBookmark(i)+1)%10);
        TextOut(DC, x+2, y, Str, strlen(Str));
        delete[] Str;
      }
      else if (LineNumbersVisible)
      {
        char* Str = IntToStr(i+1);
        int k = (GutterWidth-6)/GutterCellWidth;
        if (k > 0)
        {
          if ((int)strlen(Str) > k)
            strcpy(Str, &Str[strlen(Str)-k]);
          SetTextColor(DC, Colors.GutterText);
          TextOut(DC, 2, y, Str, strlen(Str));
        }
        delete[] Str;
      }
      /* Clean up */
      DeleteObject(SelectObject(DC, OldPen));
      DeleteObject(SelectObject(DC, OldBrush));
      Y += CellHeight;
    }
    DeleteObject(SelectObject(DC, OldFont));
  }
}

void LambdaEditor::PaintLine(HDC DC, int Index, int First, int Last)
{
  if (Document != NULL)
  {
    /* Calculate the update region */
    RECT R;
    R.left = GutterWidth+1+(First-Document->GetLeftCol())*CellWidth;
    R.top = (Index-Document->GetTopRow())*CellHeight+1;
    R.right = R.left+(Last-First+1)*CellWidth;
    R.bottom = R.top+CellHeight;
    /* If the update region is visible in the window */
    if (R.right >= 0 && R.bottom >= 0 && R.left <= Width && R.top <= Height)
    {
      Last = Min(Last, Document->GetLineSize(Index)-1);
      int SelStart = 0;
      int SelEnd = 0;
      /* Determine which part of the line is selected */
      if (PointCmp(Document->GetSelStart(), Document->GetSelEnd()) != 0)
      {
        if (Index == Document->GetSelStart().y && Index == Document->GetSelEnd().y)
        {
          SelStart = Max(0, Document->GetSelStart().x-Document->GetLeftCol());
          SelEnd = Max(0, Document->GetSelEnd().x-Document->GetLeftCol());
        }
        else if (Document->GetSelStart().y == Index)
        {
          SelStart = Max(0, Document->GetSelStart().x-Document->GetLeftCol());
          SelEnd = Max(0, Last-Document->GetLeftCol()+2);
        }
        else if (Document->GetSelEnd().y == Index)
        {
          SelStart = 0;
          SelEnd = Max(0, Document->GetSelEnd().x-Document->GetLeftCol());
        }
        else if (Index > Document->GetSelStart().y && Index < Document->GetSelEnd().y)
        {
          SelStart = 0;
          SelEnd = Max(0, Last-Document->GetLeftCol()+2);
        }
      }
      SetBkMode(DC, OPAQUE);
      /* Draws the background */
      HBRUSH OldBrush = (HBRUSH)SelectObject(DC, CreateSolidBrush(Colors.Window));
      HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Window));
      Rectangle(DC, R.left, R.top, R.left+SelStart*CellWidth, R.bottom);
      Rectangle(DC, R.left+SelEnd*CellWidth, R.top, R.right, R.bottom);
      DeleteObject(SelectObject(DC, OldPen));
      DeleteObject(SelectObject(DC, OldBrush));
      /* Draws the selection background */
      OldBrush = (HBRUSH)SelectObject(DC, CreateSolidBrush(Colors.Highlight));
      OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Highlight));
      Rectangle(DC, R.left+SelStart*CellWidth, R.top, R.left+SelEnd*CellWidth, R.bottom);
      DeleteObject(SelectObject(DC, OldPen));
      DeleteObject(SelectObject(DC, OldBrush));
      int CharEnd = R.left;
      if (Index < Document->GetLineCount() && First <= Last)
      {
        /* Draws the characters */
        HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &Font));
        const char* Str = Document->GetLine(Index);
        SetBkMode(DC, TRANSPARENT);
        for (int i = First; i <= Last; i++)
        {
          if (PointCmp(Point(i, Index), Document->GetSelStart()) >= 0 && PointCmp(Point(i, Index), Document->GetSelEnd()) < 0)
            SetTextColor(DC, Colors.HighlightText);
          else
            SetTextColor(DC, Colors.WindowText);
          TextOut(DC, CharEnd, R.top, &Str[i], 1);
          CharEnd += CellWidth;
        }
        DeleteObject(SelectObject(DC, OldFont));
      }
      /* Draws the right margin */
      if (MarginVisible)
      {
        R.left = GutterWidth+1+(MarginSize-Document->GetLeftCol())*CellWidth;
        HBRUSH OldPen = (HBRUSH)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Margin));
        MoveToEx(DC, R.left, R.top, NULL);
        LineTo(DC, R.left, R.bottom);
        DeleteObject(SelectObject(DC, OldPen));
      }
    }
  }
}

void LambdaEditor::ReplaceTabs(string* Str)
{
  /* Replace all tab characters with spaces */
  int Index = Str->find('\t');
  while (Index >= 0)
  {
    Str->replace(Index, 1, Spaces(IndentSize(Index)));
    Index = Str->find('\t', Index+IndentSize(Index));
  }
}

void LambdaEditor::SetHScrollBar(int Pos, int Range)
{
  SCROLLINFO ScrollInfo;
  ScrollInfo.cbSize = sizeof(SCROLLINFO);
  ScrollInfo.fMask = SIF_RANGE | SIF_POS;
  ScrollInfo.nPos = Pos;
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = Range;
  SetScrollInfo(Handle, SB_HORZ, &ScrollInfo, TRUE);
}

void LambdaEditor::SetVScrollBar(int Pos, int Range)
{
  SCROLLINFO ScrollInfo;
  ScrollInfo.cbSize = sizeof(SCROLLINFO);
  ScrollInfo.fMask = SIF_RANGE | SIF_POS;
  ScrollInfo.nPos = Pos;
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = Range;
  SetScrollInfo(Handle, SB_VERT, &ScrollInfo, TRUE);
}

void LambdaEditor::Update(int Message, int wParam, int lParam)
{
  switch (Message)
  {
    case LOADING_STRING:
      ReplaceTabs((string*)wParam);
      break;
    case UPDATE_FILENAME:
      if (DocumentRenamedProc != NULL)
        (*DocumentRenamedProc)(DocumentIndex(), Document->GetFileName().c_str());
      break;
    case UPDATE_CARET:
      AdjustView();
      PaintCaret();
      if (CursorPosChangedProc != NULL)
        (*CursorPosChangedProc)();
      break;
    case UPDATE_LINES:
      if (Document != NULL && UpdateCount == 0)
      {
        RECT R;
        R.left = 0;
        R.top = 1+(wParam-Document->GetTopRow())*CellHeight;
        R.right = Width;
        R.bottom = 1+(lParam-Document->GetTopRow()+1)*CellHeight;
        InvalidateRect(Handle, &R, FALSE);
      }
      break;
    case UPDATE_STATE:
      if (StateChangedProc != NULL)
        (*StateChangedProc)();
      break;
    case UPDATE_STRING:
      ReplaceTabs((string*)wParam);
      if (InsertStringProc != NULL)
        (*InsertStringProc)((string*)wParam);
      break;
    case UPDATE_VIEW:
      Invalidate();
      PaintCaret();
      UpdateScrollbars();
      break;
  }
}

void LambdaEditor::UpdateCellSize()
{
  SIZE Size;
  HDC DC = GetWindowDC(Handle);
  /* Calculate the editor's cell size */
  HFONT OldFont = (HFONT)SelectObject(DC, EasyCreateFont(DC, &Font));
  GetTextExtentPoint32(DC, "W", 1, &Size);
  CellWidth = Max(1, Size.cx);
  CellHeight = Max(1, Size.cy);
  /* Calculate the gutter's cell size */
  DeleteObject(SelectObject(DC, EasyCreateFont(DC, &GutterFont)));
  GetTextExtentPoint32(DC, "W", 1, &Size);
  GutterCellWidth = Max(1, Size.cx);
  GutterCellHeight = Max(1, Size.cy);
  /* Calculate the caret size */
  CaretWidth = 1;
  CaretHeight = CellHeight;
  /* Adjust editor and gutter cells to match height */
  if (GutterCellHeight > CellHeight)
    CellHeight = GutterCellHeight;
  /* Clean up */
  DeleteObject(SelectObject(DC, OldFont));
  ReleaseDC(Handle, DC);
  /* Update the editor */
  UpdateSize(Width, Height);
  PaintCaret();
}

void LambdaEditor::UpdateSize(int NewWidth, int NewHeight)
{
  /* Stores the size of the window */
  Width = NewWidth;
  Height = NewHeight;
  /* Calculate the amount of visible cells */
  if (CellWidth > 0)
    VisibleCols = (Width-GutterWidth-1)/CellWidth;
  else
    VisibleCols = 0;
  if (CellHeight > 0)
    VisibleRows = Height/CellHeight;
  else
    VisibleRows = 0;
  UpdateScrollbars();
}

void LambdaEditor::UpdateScrollbars()
{
  if (Document != NULL)
  {
    int MaxSize = Document->MaxLineSize();
    if (MaxSize > VisibleCols)
      SetHScrollBar(Document->GetLeftCol(), MaxSize-VisibleCols);
    else
      SetHScrollBar(0, 0);
    if (Document->GetLineCount() > VisibleRows)
      SetVScrollBar(Document->GetTopRow(), Document->GetLineCount()-VisibleRows);
    else
      SetVScrollBar(0, 0);
  }
}

// PRIVATE EVENT FUNCTIONS -----------------------------------------------------

int LambdaEditor::Char(int c)
{
  if (Document != NULL && c >= 32 && c != 127 && c <= 254)
  {
    if (OverwriteSelection && Document->IsSelected())
      Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
    else if (OverwriteMode && Document->GetCaretPos().x < Document->GetLineSize(Document->GetCaretPos().y))
      Document->DeleteString(Document->GetCaretPos(), Point(Document->GetCaretPos().x+1, Document->GetCaretPos().y));
    Document->InsertChar(Document->GetCaretPos(), (char)c);
    return 0;
  }
  return 1;
}

void LambdaEditor::DoubleClick(int Keys, int x, int y)
{
  if (Document != NULL)
  {
    if (DoubleClickLine || x <= GutterWidth)
    {
      Document->SetCaretPos(Document->GetLineSize(Document->GetCaretPos().y), Document->GetCaretPos().y);
      Document->Select(Point(0, Document->GetCaretPos().y), Point(Document->GetLineSize(Document->GetCaretPos().y), Document->GetCaretPos().y));
    }
    else
    {
      TPoint Start = {Document->GetWordStart(Document->GetCaretPos(), SpaceSeparatedWords), Document->GetCaretPos().y};
      TPoint End = {Document->GetWordEnd(Document->GetCaretPos(), SpaceSeparatedWords), Document->GetCaretPos().y};
      Document->SetCaretPos(End.x, End.y);
      Document->Select(Start, End);
    }
    if (CursorPosChangedProc != NULL)
      (*CursorPosChangedProc)();
  }
}

void LambdaEditor::HScroll(int ScrollCode, int Pos)
{
  if (Document != NULL)
  {
    int NewPos = Document->GetLeftCol();
    switch (ScrollCode)
    {
      case SB_LINELEFT: NewPos -= Pos; break;
      case SB_LINERIGHT: NewPos += Pos; break;
      case SB_PAGELEFT: NewPos -= VisibleCols*Pos; break;
      case SB_PAGERIGHT: NewPos += VisibleCols*Pos; break;
      case SB_TOP: NewPos = 0; break;
      case SB_BOTTOM: NewPos = Document->MaxLineSize()-VisibleCols; break;
      case SB_THUMBPOSITION: 
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = Max(0, Min(NewPos, Document->MaxLineSize()-VisibleCols));
    Document->SetLeftCol(NewPos);
  }
}

int LambdaEditor::KeyDown(int Key)
{
  if (Document != NULL)
  {
//    bool Alt = (GetKeyState(VK_MENU) & 0x8000) != 0;      // 16th bit is 1
    bool Ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;  // 16th bit is 1
    bool Shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;   // 16th bit is 1
    switch (Key)
    {
      case VK_LEFT:
        if (Document->GetCaretPos().x > 0)
        {
          Document->SetCaretPos(Document->GetCaretPos().x-1, Document->GetCaretPos().y, Shift, true);
          if (Ctrl)
            Document->SetCaretPos(Document->GetWordStart(Document->GetCaretPos(), SpaceSeparatedWords), Document->GetCaretPos().y, Shift, true);
        }
        else if (Document->GetCaretPos().y > 0)
          Document->SetCaretPos(Document->GetLineSize(Document->GetCaretPos().y-1), Document->GetCaretPos().y-1, Shift, true);
        return 0;
      case VK_RIGHT:
        if (Document->GetCaretPos().x < Document->GetLineSize(Document->GetCaretPos().y))
        {
          Document->SetCaretPos(Document->GetCaretPos().x+1, Document->GetCaretPos().y, Shift, true);
          if (Ctrl)
            Document->SetCaretPos(Document->GetWordEnd(Document->GetCaretPos(), SpaceSeparatedWords), Document->GetCaretPos().y, Shift, true);
        }
        else if (Document->GetCaretPos().y < Document->GetLineCount()-1)
          Document->SetCaretPos(0, Document->GetCaretPos().y+1, Shift, true);
        return 0;
      case VK_DOWN:
        if (Document->GetCaretPos().y < Document->GetLineCount()-1)
          Document->SetCaretPos(Document->GetCaretPos().x, Document->GetCaretPos().y+1, Shift, true);
        return 0;
      case VK_UP:
        if (Document->GetCaretPos().y > 0)
          Document->SetCaretPos(Document->GetCaretPos().x, Document->GetCaretPos().y-1, Shift, true);
        return 0;
      case VK_HOME:
        if (Document->GetCaretPos().x > 0 || Document->GetCaretPos().y > 0)
          if (Ctrl)
            Document->SetCaretPos(0, 0, Shift, true);
          else
            Document->SetCaretPos(0, Document->GetCaretPos().y, Shift, true);
        return 0;
      case VK_END:
        if (Document->GetCaretPos().x < Document->GetLineSize(Document->GetCaretPos().y) || Document->GetCaretPos().y < Document->GetLineCount())
          if (Ctrl)
            Document->SetCaretPos(Document->GetLineSize(Document->GetLineCount()-1), Document->GetLineCount()-1, Shift, true);
          else
            Document->SetCaretPos(Document->GetLineSize(Document->GetCaretPos().y), Document->GetCaretPos().y, Shift, true);
        return 0;
      case VK_BACK:
        if (Document->IsSelected())
          Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
        else if (Ctrl && Document->GetCaretPos().x != Document->GetWordStart(Document->GetCaretPos(), SpaceSeparatedWords))
          Document->DeleteString(Point(Document->GetWordStart(Document->GetCaretPos(), SpaceSeparatedWords), Document->GetCaretPos().y), Document->GetCaretPos());
        else if (Document->GetCaretPos().x > 0 || Document->GetCaretPos().y > 0)
          if (Document->GetCaretPos().x == 0 && Document->GetCaretPos().y > 0)
            Document->DeleteString(Point(Document->GetLineSize(Document->GetCaretPos().y-1), Document->GetCaretPos().y-1), Document->GetCaretPos());
          else
            Document->DeleteString(Point(Document->GetCaretPos().x-1, Document->GetCaretPos().y), Document->GetCaretPos());
        return 0;
      case VK_DELETE:
        if (Document->IsSelected())
          Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
        else if (Document->GetCaretPos().x < Document->GetLineSize(Document->GetCaretPos().y) || Document->GetCaretPos().y < Document->GetLineCount())
          if (Document->GetCaretPos().x == Document->GetLineSize(Document->GetCaretPos().y) && Document->GetCaretPos().y < Document->GetLineCount()-1)
            Document->DeleteString(Document->GetCaretPos(), Point(0, Document->GetCaretPos().y+1));
          else
            Document->DeleteString(Document->GetCaretPos(), Point(Document->GetCaretPos().x+1, Document->GetCaretPos().y));
        return 0;
      case VK_RETURN:
      {
        string Str = "\r\n";
        if (OverwriteSelection && Document->IsSelected())
          Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
        if (AutoIndent)
          Str.append(Document->GetLineIndentation(Document->GetCaretPos().y), ' ');
        Document->InsertString(Document->GetCaretPos(), Str);
        return 0;
      }
      case VK_TAB:
      {
        if (OverwriteMode && Document->GetCaretPos().x < Document->GetLineSize(Document->GetCaretPos().y))
          Document->SetCaretPos(Min(Document->GetLineSize(Document->GetCaretPos().y), Document->GetCaretPos().x+IndentSize(Document->GetCaretPos().x)), Document->GetCaretPos().y, true);
        else
        {
          if (Ctrl)
            Document->IndentLines(Document->GetSelStart().y, Document->GetSelEnd().y, TabSize);
          else if (Shift)
            Document->UnindentLines(Document->GetSelStart().y, Document->GetSelEnd().y, TabSize);
          else if (TabIndentsLines && Document->IsSelected())
            Document->IndentLines(Document->GetSelStart().y, Document->GetSelEnd().y, TabSize);
          else if (!Ctrl && !Shift)
          {
            if (OverwriteSelection && Document->IsSelected())
              Document->DeleteString(Document->GetSelStart(), Document->GetSelEnd());
            Document->InsertString(Document->GetCaretPos(), Spaces(IndentSize(Document->GetCaretPos().x)));
          }
        }
        return 0;
      }
    }
  }
  return 1;
}

void LambdaEditor::MouseWheel(int Keys, int Delta, int x, int y)
{
  if (Document != NULL)
  {
    UINT NumLines = 3;
    if (DefaultScrollLineCount)
      SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &NumLines, 0);
    else
      NumLines = ScrollLineCount;
    if ((GetKeyState(VK_MENU) & 0x8000)) // Alt Key
    {
      if (Keys & MK_CONTROL)
        HScroll(Delta < 0 ? SB_PAGERIGHT : SB_PAGELEFT, 1);
      else
        HScroll(Delta < 0 ? SB_LINERIGHT : SB_LINELEFT, NumLines);
    }
    else
    {
      if (Keys & MK_CONTROL)
        VScroll(Delta < 0 ? SB_PAGEDOWN : SB_PAGEUP, 1);
      else
        VScroll(Delta < 0 ? SB_LINEDOWN : SB_LINEUP, NumLines);
    }
  }
}

void LambdaEditor::Paint()
{
  if (GetUpdateRect(Handle, NULL, 0) != 0)
  {
    PAINTSTRUCT PS;
    HDC DC = BeginPaint(Handle, &PS);
    if (DC != NULL)
    {
      if (Document != NULL)
      {
        AdjustGutterSize();
        /* Calculates lines and columns of cells that needs to be painted */
        int Left = Document->GetLeftCol()+Max(0,PS.rcPaint.left-GutterWidth-1)/CellWidth;
        int Top = Document->GetTopRow()+Max(0,PS.rcPaint.top-1)/CellHeight;
        int Right = Document->GetLeftCol()+Max(0,PS.rcPaint.right-GutterWidth-1)/CellWidth;
        int Bottom = Document->GetTopRow()+Max(0,PS.rcPaint.bottom-1)/CellHeight;
        /* Paint top 1 pixel margin */
        if (PS.rcPaint.top == 0)
        {
          HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Gutter));
          MoveToEx(DC, 0, 0, NULL);
          LineTo(DC, GutterWidth-2, 0);
          DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT))));
          MoveToEx(DC, GutterWidth-2, 0, NULL);
          LineTo(DC, GutterWidth-1, 0);
          DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW))));
          MoveToEx(DC, GutterWidth-1, 0, NULL);
          LineTo(DC, GutterWidth, 0);
          DeleteObject(SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Window)));
          MoveToEx(DC, GutterWidth, 0, NULL);
          LineTo(DC, Width, 0);
          DeleteObject(SelectObject(DC, OldPen));
        }
        /* Paints gutter */
        if (GutterWidth > 0)
          PaintGutter(DC, Top, Bottom);
        /* Paint left 1 pixel margin */
        if (PS.rcPaint.left <= GutterWidth)
        {
          HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 1, Colors.Window));
          MoveToEx(DC, GutterWidth, 0, NULL);
          LineTo(DC, GutterWidth, Height);
          DeleteObject(SelectObject(DC, OldPen));
        }
        /* Paints lines */
        for (int i = Top; i <= Bottom; i++)
          PaintLine(DC, i, Left, Right);
      }
      else
      {
        /* Paints empty window */
        HPEN OldPen = (HPEN)SelectObject(DC, CreatePen(PS_SOLID, 0, Colors.Window));
        HBRUSH OldBrush = (HBRUSH)SelectObject(DC, CreateSolidBrush(Colors.Window));
        Rectangle(DC, PS.rcPaint.left, PS.rcPaint.top, PS.rcPaint.right, PS.rcPaint.bottom);
        DeleteObject(SelectObject(DC, OldPen));
        DeleteObject(SelectObject(DC, OldBrush));
      }
      EndPaint(Handle, &PS);
    }
  }
}

void LambdaEditor::VScroll(int ScrollCode, int Pos)
{
  if (Document != NULL)
  {
    int NewPos = Document->GetTopRow();
    switch (ScrollCode)
    {
      case SB_LINEUP: NewPos -= Pos; break;
      case SB_LINEDOWN: NewPos += Pos; break;
      case SB_PAGEUP: NewPos -= VisibleRows*Pos; break;
      case SB_PAGEDOWN: NewPos += VisibleRows*Pos; break;
      case SB_TOP: NewPos = 1; break;
      case SB_BOTTOM: NewPos = Document->GetLineCount()-VisibleRows; break;
      case SB_THUMBPOSITION: 
      case SB_THUMBTRACK: NewPos = Pos; break;
    }
    NewPos = Max(0, Min(NewPos, Document->GetLineCount()-VisibleRows));
    Document->SetTopRow(NewPos);
  }
}

// PRIVATE WINAPI FUNCTIONS ----------------------------------------------------

HWND LambdaEditor::CreateWnd(HWND hParent)
{
  WNDCLASSEX WndClass;
  /* Specify the window class information */
  if (!GetClassInfoEx(NULL, ClassName, &WndClass))
  {
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = ClassName;
    WndClass.hInstance = GetModuleHandle(0);
    WndClass.lpfnWndProc = WindowProc;
    WndClass.style = CS_DBLCLKS	;
    WndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    WndClass.hIcon = 0;
    WndClass.hIconSm = 0;
    WndClass.hCursor = NULL;
    WndClass.lpszMenuName = 0;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    if (!RegisterClassEx(&WndClass))
      return NULL;
  }
  /* Creates the window */
  HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, ClassName, NULL,
              WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP,
              0, 0, 500, 300, hParent, NULL, GetModuleHandle(NULL), NULL);
  SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
  return hWnd;
}

LRESULT CALLBACK LambdaEditor::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  LambdaEditor* Editor = (LambdaEditor*)GetWindowLong(hWnd, GWL_USERDATA);
  static bool CancelMenuKey = false;
  int Result = 1;
  switch(Msg)
  {
    case WM_CHAR:
    {
      Result = Editor->Char((int)wParam);
      break;
    }
    case WM_DROPFILES:
    {
      return SendMessage(GetParent(hWnd), Msg, wParam, lParam);
    }
    case WM_ERASEBKGND:
    {
      return 1;
    }
    case WM_FONTCHANGE:
    {
      Editor->UpdateCellSize();
      Editor->Invalidate();
      return 0;
    }
    case WM_GETDLGCODE:
    {
      return DLGC_WANTALLKEYS;
    }
    case WM_HSCROLL:
    {
      int Pos = 1;
      /* Get the tracker position, the one received in WParam is too small */
      if (LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK)
      {
        SCROLLINFO ScrollInfo;
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_HORZ, &ScrollInfo);
        Pos = ScrollInfo.nTrackPos;
      }
      Editor->HScroll((short)LOWORD(wParam), Pos);
      return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      if (GetFocus() != hWnd)
        SetFocus(hWnd);
      if (GetCapture() != hWnd)
        SetCapture(hWnd);
      if (Editor->Document != NULL && (Msg != WM_RBUTTONDOWN || !Editor->Document->IsSelected()))
      {
        TPoint p = Editor->MouseToCaret((short)LOWORD(lParam), (short)HIWORD(lParam));
        Editor->Document->SetCaretPos(p.x, p.y, (wParam & MK_SHIFT) != 0, true);
      }
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      ReleaseCapture();
      /* Show context menu */
      POINT p;
      if (Msg == WM_RBUTTONUP && GetCursorPos(&p))
        SendMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)hWnd, MAKELPARAM(p.x, p.y));
      return 0;
    }
    case WM_LBUTTONDBLCLK:
    {
      Editor->DoubleClick(wParam | MK_LBUTTON, (short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
      Result = Editor->KeyDown(wParam);
      break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
      if (CancelMenuKey)
        CancelMenuKey = false;
      break;
    }
    case WM_KILLFOCUS:
    {
      DestroyCaret();
      Editor->CaretVisible = false;
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      if ((short)LOWORD(lParam) <= Editor->GutterWidth || Editor->Document == NULL)
        SetCursor(Editor->ArrowCursor);
      else
        SetCursor(Editor->BeamCursor);
      if (Editor->Document != NULL && wParam & MK_LBUTTON && !(wParam & MK_RBUTTON) && GetCapture() == hWnd)
      {
        TPoint p = Editor->MouseToCaret((short)LOWORD(lParam), (short)HIWORD(lParam));
        Editor->Document->SetCaretPos(p.x, p.y, true);
      }
      return 0;
    }
    case WM_MOUSEWHEEL:
    {
      Editor->MouseWheel(LOWORD(wParam), (short)HIWORD(wParam), (short)LOWORD(lParam), (short)HIWORD(lParam));
      CancelMenuKey = true;
      return 0;
    }
    case WM_PAINT:
    {
      Editor->Paint();
      return 0;
    }
    case WM_SETFOCUS:
    {
      CreateCaret(hWnd, NULL, Editor->CaretWidth, Editor->CaretHeight);
      Editor->PaintCaret();
      return 0;
    }
    case WM_SIZE:
    {
      if (Editor != NULL)
        if (LOWORD(lParam) != Editor->Width || HIWORD(lParam) != Editor->Height)
          Editor->UpdateSize((short)LOWORD(lParam), (short)HIWORD(lParam));
      return 0;
    }
    case WM_STYLECHANGED:
    case WM_SYSCOLORCHANGE:
    {
      Editor->Paint();
      return 0;
    }
    case WM_VSCROLL:
    {
      int Pos = 1;
      /* Get the tracker position, the one received in WParam is too small */
      if (LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK)
      {
        SCROLLINFO ScrollInfo;
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);
        Pos = ScrollInfo.nTrackPos;
      }
      Editor->VScroll((short)LOWORD(wParam), Pos);
      return 0;
    }
  }
  if (Result == 0)
    return Result;
  return DefWindowProc(hWnd, Msg, wParam, lParam);
}
