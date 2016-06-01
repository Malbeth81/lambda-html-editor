#ifndef EXPLORER_H
#define	EXPLORER_H

#include <windows.h>
#include <commctrl.h>
#include <string>
using namespace std;
#include "../models/observable.h"

#define WM_FILEOPENED WM_USER+160  // wParam = LPCSTR, return 1 if file is opened

class ProjectExplorer {
  public :
    int AutoHideDelay;
    bool SingleClickOpen;
    bool RenameOnClick;

    /* Public functions */
    ProjectExplorer(HWND hParent);
    ~ProjectExplorer();
    void AddDirectory(string Folder, string FileName);
    void AddFile(string Folder, string Name);
    void AddFolder(string Folder, string Name);
    void Clear();
    void DeleteSelected();
    void EditSelected();
    bool GetAutoHide();
    string GetFileName();
    HWND GetHandle();
    bool GetHide();
    string GetProjectName();
    string GetSelectedFile();
    string GetSelectedFolder();
    bool IsModified();
    bool IsVisible();
    bool LoadFromFile(const char* FileName);
    void RenameFile(string OldName, string NewName);
    bool SaveToFile(const char* FileName);
    void SetAutoHide(bool autoHide);
    void SetHide(bool hide);
    void SetProjectName(string Name);
    void SetSize(RECT* R);
  private:
    bool AutoHide;
    LONG DefaultWndProc;
    int Left;
    int Top;
    string FileName;
    HWND Handle;
    bool Hide;
    bool Modified;
    string ProjectName;

    /* GUI functions */
    HTREEITEM AddTreeViewItem(HTREEITEM Parent, HTREEITEM Prev, string Text, int Icon, int Param);
    HTREEITEM AddTreeViewFile(HTREEITEM Parent, string Name);
    HTREEITEM AddTreeViewFolder(HTREEITEM Parent, string Name);
    int CountTreeViewFiles(HTREEITEM Parent);
    void DeleteTreeViewItem(HTREEITEM Item);
    bool IsChildOf(HTREEITEM Item, HTREEITEM ChildItem);
    HTREEITEM FindTreeViewFile(HTREEITEM Parent, string Name);
    HTREEITEM FindTreeViewFolder(HTREEITEM Parent, string Name);
    HTREEITEM GetTreeViewItem(HTREEITEM Parent, string Name);
    HTREEITEM GetTreeViewItem(int x, int y);
    string GetTreeViewItemFolderName(HTREEITEM Item);
    int GetTreeViewItemIcon(HTREEITEM Item);
    string GetTreeViewItemName(HTREEITEM Item);
    int GetTreeViewItemLParam(HTREEITEM Item);
    void MoveTreeViewFile(HTREEITEM Parent, HTREEITEM Item);
    void MoveTreeViewFolder(HTREEITEM Parent, HTREEITEM Item);
    void SaveFolderToFile(ofstream& File, HTREEITEM Parent, string FolderName);
    void SelectTreeViewItem(HTREEITEM Item, int Flag);
    void SetTreeViewItemIcon(HTREEITEM Item, int Icon);
    void SetTreeViewItemName(HTREEITEM Item, string Name);
    void SetTreeViewItemLParam(HTREEITEM Item, int lParam);
    /* WINAPI functions */
    HWND CreateWnd(HWND hParent);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
