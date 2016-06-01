#ifndef DOCUMENTLIST_H
#define	DOCUMENTLIST_H

#include "document.h"

typedef struct TDocumentNode {
  PDocument Document;
  TDocumentNode* Prev;
  TDocumentNode* Next;
} *PDocumentNode;

typedef class TDocumentList {
  public :
    TDocumentList();
    ~TDocumentList();

    void Add(PDocument Document);
    void Clear();
    int Count();
    void Move(int FromIndex, int ToIndex);
    void Remove(int Index);

    PDocument operator[](int Index);
  private:
    PDocumentNode FirstNode;
    int NodeCount;

    PDocumentNode GetNode(int Index);
} *PDocumentList;

#endif
