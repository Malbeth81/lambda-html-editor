#include <fstream>
#include "stringlist.h"

void Split(string& Str1, string& Str2)
{
  int Pos = Str1.find_first_of("\r\n", 0);
  if (Pos >= 0)
  {
    int Pos2 = Str1.find_first_not_of("\r\n", Pos);
    if (Pos2 >= 0)
      Str2 = Str1.substr(Pos2);
    Str1 = Str1.substr(0, Pos);
  }
}

TStringList::TStringList()
{
  First = NULL;
  NodeCount = 0;
}

TStringList::~TStringList()
{
  Clear();
}

PStringListNode TStringList::GetNode(long Line)
{
  PStringListNode Ptr = First;
  while (Ptr != NULL && Line > 0)
  {
    Ptr = Ptr->Next;
    Line--;
  }
  return Ptr;
}

void TStringList::Add(string Str)
{
  string Str2 = "";
  PStringListNode Ptr = GetNode(NodeCount-1);
  Split(Str, Str2);
  PStringListNode Node = new TStringListNode;
  Node->Str.assign(Str);
  Node->Prev = Ptr;
  Node->Next = NULL;
  if (Ptr != NULL)
    Ptr->Next = Node;
  else
    First = Node;
  NodeCount++;
  if (Str2 != "")
    Add(Str2);
}

void TStringList::Clear()
{
  PStringListNode Ptr;
  while (First != NULL)
  {
    Ptr = First;
    First = First->Next;
    delete Ptr;
  }
  NodeCount = 0;
}

long TStringList::Count()
{
  return NodeCount;
}

void TStringList::Insert(long Line, string Str)
{
  string Str2 = "";
  PStringListNode Ptr = GetNode(Line-1);
  Split(Str, Str2);
  PStringListNode Node = new TStringListNode;
  Node->Str.assign(Str);
  Node->Prev = Ptr;
  Node->Next = NULL;
  if (Ptr != NULL)
  {
    Node->Next = Ptr->Next;
    Ptr->Next = Node;
  }
  else
    First = Node;
  NodeCount++;
  if (Str2 != "")
    Insert(Line+1, Str2);
}

bool TStringList::LoadFromFile(const char* FileName)
{
  bool Result = false;
  ifstream File(FileName, ios::in);
  if (File.is_open())
  {
    Clear();
    string Line;
    while (!File.eof())
    {
      getline(File, Line);
      Add(Line);
    }
    File.close();
    Result = true;
  }
  return Result;
}

void TStringList::Remove(long Line)
{
  PStringListNode Ptr = GetNode(Line);
  if (Ptr != NULL)
  {
    if (Ptr->Prev != NULL)
      Ptr->Prev->Next = Ptr->Next;
    else
      First = Ptr->Next;
    if (Ptr->Next != NULL)
      Ptr->Next->Prev = Ptr->Prev;
    delete Ptr;
    NodeCount--;
  }
}

bool TStringList::SaveToFile(const char* FileName)
{
  bool Result = false;
  ofstream File(FileName, ios::out);
  if (File.is_open())
  {
    PStringListNode Ptr = First;
    while (Ptr != NULL)
    {
      File.write(Ptr->Str.c_str(), Ptr->Str.size());
      File.write("\n", 1);
      Ptr = Ptr->Next;
    }
    File.close();
    Result = true;
  }
  return Result;
}

string& TStringList::operator[](long Line)
{
  PStringListNode Ptr = First;
  while (Ptr != NULL && Line > 0)
  {
    Ptr = Ptr->Next;
    Line--;
  }
  if (Ptr != NULL)
    return Ptr->Str;
  return *(new string);
}
