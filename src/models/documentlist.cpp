#include "documentlist.h"

TDocumentList::TDocumentList()
{
  FirstNode = NULL;
  NodeCount = 0;
}

TDocumentList::~TDocumentList()
{
  Clear();
}

void TDocumentList::Add(PDocument Document)
{
  /* Finds the last Node */
  PDocumentNode Ptr = FirstNode;
  if (Ptr != NULL)
    while (Ptr->Next != NULL)
      Ptr = Ptr->Next;
  /* Creates new Node */
  PDocumentNode Node = new TDocumentNode;
  Node->Document = Document;
  Node->Prev = Ptr;
  Node->Next = NULL;
  if (Ptr != NULL)
    Ptr->Next = Node;
  else
    FirstNode = Node;
  NodeCount++;
}

void TDocumentList::Clear()
{
  PDocumentNode Ptr;
  while (FirstNode != NULL)
  {
    Ptr = FirstNode;
    FirstNode = FirstNode->Next;
    delete Ptr->Document;
    delete Ptr;
  }
  NodeCount = 0;
}

int TDocumentList::Count()
{
  return NodeCount;
}

void TDocumentList::Move(int FromIndex, int ToIndex)
{
  if (ToIndex != FromIndex)
  {
    PDocumentNode Node = GetNode(FromIndex);
    PDocumentNode Ptr = GetNode(ToIndex);
    if (Node != NULL && Ptr != NULL)
    {
      /* Removes the Node */
      if (Node->Prev != NULL)
        Node->Prev->Next = Node->Next;
      else
        FirstNode = Node->Next;
      if (Node->Next != NULL)
        Node->Next->Prev = Node->Prev;
      /* Inserts the Node */
      Node->Prev = (ToIndex > FromIndex ? Ptr : Ptr->Prev);
      Node->Next = (ToIndex > FromIndex ? Ptr->Next : Ptr);
      if (Node->Prev != NULL)
        Node->Prev->Next = Node;
      else
        FirstNode = Node;
      if (Node->Next != NULL)
        Node->Next->Prev = Node;
    }
  }
}

void TDocumentList::Remove(int Index)
{
  PDocumentNode Ptr = GetNode(Index);
  /* Removes Node */
  if (Ptr != NULL)
  {
    if (Ptr->Prev != NULL)
      Ptr->Prev->Next = Ptr->Next;
    else
      FirstNode = Ptr->Next;
    if (Ptr->Next != NULL)
      Ptr->Next->Prev = Ptr->Prev;
    delete Ptr->Document;
    delete Ptr;
    NodeCount--;
  }
}

PDocument TDocumentList::operator[](int Index)
{
  PDocumentNode Ptr = GetNode(Index);
  if (Ptr != NULL)
    return Ptr->Document;
  return NULL;
}


PDocumentNode TDocumentList::GetNode(int Index)
{
  /* Finds the Node */
  PDocumentNode Ptr = FirstNode;
  while (Ptr != NULL && Index > 0)
  {
    Ptr = Ptr->Next;
    Index --;
  }
  return Ptr;
}
