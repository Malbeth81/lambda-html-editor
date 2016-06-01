#ifndef STRINGLIST_H
#define	STRINGLIST_H

#include <string>
using namespace std;

typedef struct TStringListNode {
  string Str;
  TStringListNode* Prev;
  TStringListNode* Next;
} *PStringListNode;

typedef class TStringList {
  private:
    PStringListNode First;
    long NodeCount;
    
    PStringListNode GetNode(long Line);
  public :
    TStringList();
    ~TStringList();

    void Add(string Str);
    void Clear();
    long Count();
    void Insert(long Line, string Str);
    bool LoadFromFile(const char* FileName);
    void Remove(long Line);
    bool SaveToFile(const char* FileName);
    string& operator[](long Line);
} *PStringList;


#endif
