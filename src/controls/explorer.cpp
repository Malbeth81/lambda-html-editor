#include <fstream>
#include "explorer.h"
#include "../tools.h"
#include "../resource.h"

#define ICON_SIZE 16

LONG DefEditorWndProc;
LRESULT CALLBACK EditorWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_GETDLGCODE:
    {
      return DLGC_WANTALLKEYS;
    }
  }
  return CallWindowProc((WNDPROC)DefEditorWndProc, hWnd, Msg, wParam, lParam);
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

ProjectExplorer::ProjectExplorer(HWND hParent)
{
  AutoHide = false;
  AutoHideDelay = 2000;
  Hide = false;
  SingleClickOpen = false;
  RenameOnClick = false;
  FileName = "";
  Modified = false;
  ProjectName = "";
  Handle = CreateWnd(hParent);
  if (Handle != NULL)
    Clear();
}

ProjectExplorer::~ProjectExplorer()
{
  /* Restore default WndProc */
  SetWindowLong(Handle, GWL_WNDPROC, DefaultWndProc);
}

void ProjectExplorer::AddDirectory(string Folder, string FileName)
{
  /* Adds the directory */
  HTREEITEM Parent = FindTreeViewFolder(TreeView_GetRoot(Handle), Folder);
  Parent = AddTreeViewFolder(Parent, FileName.substr(FileName.rfind('\\')+1));
  WIN32_FIND_DATA FindData;
  HANDLE Handle = FindFirstFile((FileName+"\\*.*").c_str(), &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    do if (FindData.cFileName[0] != '.')
    {
      /* Adds items inside the directory */
      if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        AddDirectory(GetTreeViewItemFolderName(Parent), FileName+"\\"+FindData.cFileName);
      else
        AddTreeViewFile(Parent, FileName+"\\"+FindData.cFileName);
    }
    while (FindNextFile(Handle, &FindData) != 0);
    FindClose(Handle);
  }
  Modified = true;
}

void ProjectExplorer::AddFile(string Folder, string Name)
{
  if (Name.size() > 0)
  {
    HTREEITEM Parent = FindTreeViewFolder(TreeView_GetRoot(Handle), Folder);
    AddTreeViewFile(Parent, Name);
    Modified = true;
  }
}

void ProjectExplorer::AddFolder(string Folder, string Name)
{
  if (Name.size() > 0)
  {
    HTREEITEM Parent = FindTreeViewFolder(TreeView_GetRoot(Handle), Folder);
    AddTreeViewFolder(Parent, Name);
    Modified = true;
  }
}

void ProjectExplorer::Clear()
{
  DeleteTreeViewItem(TreeView_GetRoot(Handle));
  AddTreeViewItem(TVI_ROOT, TVI_LAST, ProjectName, 1, 0);
}

void ProjectExplorer::DeleteSelected()
{
  /* Find the Node */
  HTREEITEM Item = TreeView_GetSelection(Handle);
  /* Delete the Node */
  if (Item != NULL && Item != TreeView_GetRoot(Handle))
    DeleteTreeViewItem(Item);
}

void ProjectExplorer::EditSelected()
{
  /* Find the Node */
  HTREEITEM Item = TreeView_GetSelection(Handle);
  /* Edit the Node */
  if (Item != NULL)
  {
    HWND Editor = TreeView_EditLabel(Handle, Item);
    if (Editor != NULL)
      DefEditorWndProc = SetWindowLong(Editor, GWL_WNDPROC, (LONG)&EditorWndProc);
  }
}

bool ProjectExplorer::GetAutoHide()
{
  return AutoHide;
}

string ProjectExplorer::GetFileName()
{
  return FileName;
}

HWND ProjectExplorer::GetHandle()
{
  return Handle;
}

bool ProjectExplorer::GetHide()
{
  return Hide;
}

string ProjectExplorer::GetProjectName()
{
  return ProjectName;
}

string ProjectExplorer::GetSelectedFile()
{
  HTREEITEM Item = TreeView_GetSelection(Handle);
  if (Item != NULL)
  {
    TV_ITEM TVItem;
    TVItem.mask = TVIF_PARAM;
    TVItem.hItem = Item;
    if (TreeView_GetItem(Handle, &TVItem) != 0 && (void*)TVItem.lParam != NULL)
      return *(string*)TVItem.lParam;
  }
  return "";
}

string ProjectExplorer::GetSelectedFolder()
{
  string Result = "";
  return GetTreeViewItemFolderName(TreeView_GetSelection(Handle));
}

bool ProjectExplorer::IsModified()
{
  return Modified;
}

bool ProjectExplorer::IsVisible()
{
  return IsWindowVisible(Handle) != 0;
}

bool ProjectExplorer::LoadFromFile(const char* FileName)
{
  ifstream File(FileName, ios::in);
  if (File.is_open())
  {
    Clear();
    string CurFolder = "";
    string Str;
    while (!File.eof())
    {
      getline(File, Str);
      if (Str.compare(0, 8, "Project=") == 0)
        SetProjectName(Str.substr(8));
      else if (Str.compare(0, 7, "Folder=") == 0)
      {
        CurFolder = Str.substr(7);
        int Pos = CurFolder.rfind('\\');
        if (Pos >= 0)
          AddFolder(CurFolder.substr(0, Pos), CurFolder.substr(Pos+1));
        else
          AddFolder("", CurFolder);
      }
      else if (Str.compare(0, 5, "File=") == 0)
      {
        if (Str.compare(5, 7, "Opened;") == 0)
        {
          string Str2 = Str.substr(12);
          AddFile(CurFolder, Str2);
          SendMessage(GetParent(Handle), WM_COMMAND, MAKEWPARAM(ID_FILE_OPENFILE, 0), 0);
        }
        else if (Str.compare(5, 7, "Closed;") == 0)
          AddFile(CurFolder, Str.substr(12));
        else
          AddFile(CurFolder, Str.substr(5));
      }
    }
    File.close();
    this->FileName.assign(FileName);
    Modified = false;
    return true;
  }
  return false;
}

void ProjectExplorer::RenameFile(string OldName, string NewName)
{
  /* Finds the File */
  HTREEITEM Item = FindTreeViewFile(TreeView_GetRoot(Handle), OldName);
  /* Renames the File */
  if (Item != NULL)
  {
    string Caption = NewName.substr(NewName.rfind('\\')+1);
    SetTreeViewItemName(Item, Caption);
    delete (string*)GetTreeViewItemLParam(Item);
    string* Str = new string(NewName);
    SetTreeViewItemLParam(Item, (int)Str);
  }
}

bool ProjectExplorer::SaveToFile(const char* FileName)
{
  ofstream File(FileName, ios::out);
  if (File.is_open())
  {
    SaveFolderToFile(File, TreeView_GetRoot(Handle), "");
    File.close();
    this->FileName.assign(FileName);
    Modified = false;
    return true;
  }
  return false;
}

void ProjectExplorer::SetAutoHide(bool autoHide)
{
  AutoHide = autoHide;
  if (AutoHide)
    SetTimer(Handle, 1, AutoHideDelay, NULL);
  else
    KillTimer(Handle, 1);
}

void ProjectExplorer::SetHide(bool hide)
{
  Hide = hide;
  /* Changes the visibility of the window */
  if (Hide && IsVisible())
    ShowWindow(Handle, SW_HIDE);
  else if (!Hide && !IsVisible())
    ShowWindow(Handle, SW_SHOW);
  /* Notifies the parent */
  RECT R;
  GetClientRect(GetParent(Handle), &R);
  PostMessage(GetParent(Handle), WM_SIZE, R.right, R.bottom);
  /* Activates timer if auto hide is on */
  if (!Hide && AutoHide)
    SetTimer(Handle, 1, AutoHideDelay, NULL);
}

void ProjectExplorer::SetProjectName(string Name)
{
  ProjectName.assign(Name);
  SetTreeViewItemName(TreeView_GetRoot(Handle), Name);
}

// PRIVATE GUI FUNCTIONS -------------------------------------------------------

HTREEITEM ProjectExplorer::AddTreeViewItem(HTREEITEM Parent, HTREEITEM Prev, string Text, int Icon, int Param)
{
  char* Caption = new char [MAX_PATH];
  strcpy(Caption, Text.c_str());
  /* Inserts the item */
  TV_ITEM TVItem;
  TV_INSERTSTRUCT Insert;
  TVItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  TVItem.pszText = Caption;
  TVItem.iImage = Icon;
  TVItem.iSelectedImage = Icon;
  TVItem.lParam = Param;
  Insert.item = TVItem;
  Insert.hInsertAfter = Prev;
  Insert.hParent = Parent;
  HTREEITEM Item = TreeView_InsertItem(Handle, &Insert);
  delete[] Caption;
  SelectTreeViewItem(Item, TVGN_CARET);
  /* Expands parent */
  TreeView_Expand(Handle, Parent, TVE_EXPAND);
  Modified = true;
  return Item;
}

HTREEITEM ProjectExplorer::AddTreeViewFile(HTREEITEM Parent, string Name)
{
  HTREEITEM Item = NULL;
  if (Parent != NULL && Name.size() > 0)
  {
    /* Find item */
    HTREEITEM Item = FindTreeViewFile(TreeView_GetRoot(Handle), Name);
    if (Item == NULL)
    {
      string Caption = Name.substr(Name.rfind('\\')+1);
      /* Finds the previous item */
      HTREEITEM PrevItem = TreeView_GetChild(Handle, Parent);
      if (PrevItem == NULL)
        PrevItem = TVI_LAST;
      else if (GetTreeViewItemIcon(PrevItem) == 0 && GetTreeViewItemName(PrevItem).compare(Caption) > 0)
        PrevItem = TVI_FIRST;
      else
      {
        HTREEITEM CurItem = TreeView_GetNextSibling(Handle, PrevItem);
        while (CurItem != NULL && (GetTreeViewItemIcon(CurItem) > 0 || GetTreeViewItemName(CurItem).compare(Caption) <= 0))
        {
          PrevItem = CurItem;
          CurItem = TreeView_GetNextSibling(Handle, CurItem);
        }
      }
      /* Adds the item */
      string* Str = new string(Name);
      Item = AddTreeViewItem(Parent, PrevItem, Caption, 0, (int)Str);
      /* Change parent icon */
      SetTreeViewItemIcon(Parent, 2);
    }
  }
  return Item;
}

HTREEITEM ProjectExplorer::AddTreeViewFolder(HTREEITEM Parent, string Name)
{
  HTREEITEM Item = NULL;
  if (Parent != NULL && Name.size() > 0)
  {
    Item = GetTreeViewItem(Parent, Name);
    if (Item == NULL)
    {
      /* Finds the previous item */
      HTREEITEM PrevItem = TreeView_GetChild(Handle, Parent);
      if (PrevItem == NULL)
        PrevItem = TVI_LAST;
      else if (GetTreeViewItemIcon(PrevItem) <= 0 || GetTreeViewItemName(PrevItem).compare(Name) > 0)
        PrevItem = TVI_FIRST;
      else
      {
        HTREEITEM CurItem = TreeView_GetNextSibling(Handle, PrevItem);
        while (CurItem != NULL && GetTreeViewItemIcon(CurItem) > 0 && GetTreeViewItemName(CurItem).compare(Name) <= 0)
        {
          PrevItem = CurItem;
          CurItem = TreeView_GetNextSibling(Handle, CurItem);
        }
      }
      /* Adds the item */
      Item = AddTreeViewItem(Parent, PrevItem, Name, 1, (int)(void*)NULL);
      /* Change parent icon */
      SetTreeViewItemIcon(Parent, 2);
    }
  }
  return Item;
}

void ProjectExplorer::DeleteTreeViewItem(HTREEITEM Item)
{
  if (Item != NULL)
  {
    /* Delete childrens */
    HTREEITEM ChildItem = TreeView_GetChild(Handle, Item);
    while (ChildItem != NULL)
    {
      HTREEITEM NextChildItem = TreeView_GetNextSibling(Handle, ChildItem);
      DeleteTreeViewItem(ChildItem);
      ChildItem = NextChildItem;
    }
    /* Delete item */
    HTREEITEM SelItem = TreeView_GetNextSibling(Handle, Item);
    if (SelItem == NULL)
      SelItem = TreeView_GetPrevSibling(Handle, Item);
    if (SelItem == NULL)
    {
      SelItem = TreeView_GetParent(Handle, Item);
      /* Change parent icon */
      SetTreeViewItemIcon(SelItem, 1);
    }
    delete (string*)GetTreeViewItemLParam(Item);
    TreeView_DeleteItem(Handle, Item);
    TreeView_SelectItem(Handle, SelItem);
    Modified = true;
  }
}

bool ProjectExplorer::IsChildOf(HTREEITEM Item, HTREEITEM ChildItem)
{
  if (Item != NULL)
    while (ChildItem != NULL)
    {
      if (ChildItem == Item)
        return true;
      ChildItem = TreeView_GetParent(Handle, ChildItem);
    }
  return false;
}

HTREEITEM ProjectExplorer::FindTreeViewFile(HTREEITEM Parent, string Name)
{
  if (Parent != NULL && Name.size() > 0)
  {
    /* Find File */
    HTREEITEM Item = TreeView_GetChild(Handle, Parent);
    while (Item != NULL)
    {
      string* Str = (string*)GetTreeViewItemLParam(Item);
      if (Str != NULL && Str->compare(Name) == 0)
        return Item;
      HTREEITEM ChildItem = FindTreeViewFile(Item, Name);
      if (ChildItem != NULL)
        return ChildItem;
      Item = TreeView_GetNextSibling(Handle, Item);
    }
  }
  return NULL;
}

HTREEITEM ProjectExplorer::FindTreeViewFolder(HTREEITEM Parent, string Name)
{
  if (Parent != NULL && Name.size() > 0)
  {
    /* Find item */
    int Pos = Name.find('\\');
    string Caption = Name.substr(0, Pos);
    HTREEITEM Item = TreeView_GetChild(Handle, Parent);
    while (Item != NULL && GetTreeViewItemName(Item).compare(Caption) != 0)
      Item = TreeView_GetNextSibling(Handle, Item);
    /* Find children */
    if (Item != NULL && Pos >= 0)
    {
      HTREEITEM ChildItem = FindTreeViewFolder(Item, Name.substr(Pos+1));
      if (ChildItem != NULL)
        return ChildItem;
    }
    return Item;
  }
  return Parent;
}

HTREEITEM ProjectExplorer::GetTreeViewItem(HTREEITEM Parent, string Name)
{
  if (Parent != NULL && Name.size() > 0)
  {
    /* Get item */
    HTREEITEM Item = TreeView_GetChild(Handle, Parent);
    while (Item != NULL && GetTreeViewItemName(Item).compare(Name) != 0)
      Item = TreeView_GetNextSibling(Handle, Item);
    return Item;
  }
  return NULL;
}

HTREEITEM ProjectExplorer::GetTreeViewItem(int x, int y)
{
  TV_HITTESTINFO HitTest;
  HitTest.pt.x = x;
  HitTest.pt.y = y;
  return TreeView_HitTest(Handle, &HitTest);
}

string ProjectExplorer::GetTreeViewItemFolderName(HTREEITEM Item)
{
  string Result = "";
  if (Item != NULL)
  {
    if (GetTreeViewItemIcon(Item) == 0)
      Item = TreeView_GetParent(Handle, Item);
    HTREEITEM Root = TreeView_GetRoot(Handle);
    while (Item != Root && Item != NULL)
    {
      if (Result.size() > 0)
        Result.insert((unsigned int)0, 1, '\\');
      Result.insert(0, GetTreeViewItemName(Item));
      Item = TreeView_GetParent(Handle, Item);
    }
  }
  return Result;
}

int ProjectExplorer::GetTreeViewItemIcon(HTREEITEM Item)
{
  if (Item != NULL)
  {
    TV_ITEM TVItem;
    TVItem.mask = TVIF_IMAGE;
    TVItem.hItem = Item;
    if (TreeView_GetItem(Handle, &TVItem))
      return TVItem.iImage;
  }
  return -1;
}

string ProjectExplorer::GetTreeViewItemName(HTREEITEM Item)
{
  string Result = "";
  if (Item != NULL)
  {
    char* Caption = new char [MAX_PATH];
    TV_ITEM TVItem;
    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = Item;
    TVItem.pszText = Caption;
    TVItem.cchTextMax = MAX_PATH;
    if (TreeView_GetItem(Handle, &TVItem))
      Result.assign(Caption);
    delete[] Caption;
  }
  return Result;
}

int ProjectExplorer::GetTreeViewItemLParam(HTREEITEM Item)
{
  if (Item != NULL)
  {
    TV_ITEM TVItem;
    TVItem.mask = TVIF_PARAM;
    TVItem.hItem = Item;
    if (TreeView_GetItem(Handle, &TVItem))
      return TVItem.lParam;
  }
  return 0;
}

void ProjectExplorer::MoveTreeViewFile(HTREEITEM Parent, HTREEITEM Item)
{
  if (Parent != NULL && Item != NULL)
  {
    /* Get the item */
    char* Caption = new char [MAX_PATH];
    TV_ITEM TVItem;
    TVItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    TVItem.hItem = Item;
    TVItem.pszText = Caption;
    TVItem.cchTextMax = MAX_PATH;
    if (TreeView_GetItem(Handle, &TVItem))
    {
      /* Finds the previous item */
      HTREEITEM PrevItem = TreeView_GetChild(Handle, Parent);
      if (PrevItem == NULL)
        PrevItem = TVI_LAST;
      else if (GetTreeViewItemIcon(PrevItem) == 0 && GetTreeViewItemName(PrevItem).compare(Caption) > 0)
        PrevItem = TVI_FIRST;
      else
      {
        HTREEITEM CurItem = TreeView_GetNextSibling(Handle, PrevItem);
        while (CurItem != NULL && (GetTreeViewItemIcon(CurItem) > 0 || GetTreeViewItemName(CurItem).compare(Caption) <= 0))
        {
          PrevItem = CurItem;
          CurItem = TreeView_GetNextSibling(Handle, CurItem);
        }
      }
      /* Inserts the item */
      TV_INSERTSTRUCT Insert;
      Insert.item = TVItem;
      Insert.hInsertAfter = PrevItem;
      Insert.hParent = Parent;
      HTREEITEM NewItem = TreeView_InsertItem(Handle, &Insert);
      /* Change parent icon */
      SetTreeViewItemIcon(Parent, 2);
      /* Expands parent */
      TreeView_Expand(Handle, Parent, TVE_EXPAND);
      /* Select new item */
      SelectTreeViewItem(NewItem, TVGN_CARET);
    }
    TreeView_DeleteItem(Handle, Item);
    delete[] Caption;
  }
}

void ProjectExplorer::MoveTreeViewFolder(HTREEITEM Parent, HTREEITEM Item)
{
  if (Parent != NULL && Item != NULL)
  {
    /* Get the item */
    char* Caption = new char [MAX_PATH];
    TV_ITEM TVItem;
    TVItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    TVItem.hItem = Item;
    TVItem.pszText = Caption;
    TVItem.cchTextMax = MAX_PATH;
    if (TreeView_GetItem(Handle, &TVItem))
    {
      /* Finds the previous item */
      HTREEITEM PrevItem = TreeView_GetChild(Handle, Parent);
      if (PrevItem == NULL)
        PrevItem = TVI_LAST;
      else if (GetTreeViewItemIcon(PrevItem) <= 0 || GetTreeViewItemName(PrevItem).compare(Caption) > 0)
        PrevItem = TVI_FIRST;
      else
      {
        HTREEITEM CurItem = TreeView_GetNextSibling(Handle, PrevItem);
        while (CurItem != NULL && GetTreeViewItemIcon(CurItem) > 0 && GetTreeViewItemName(CurItem).compare(Caption) <= 0)
        {
          PrevItem = CurItem;
          CurItem = TreeView_GetNextSibling(Handle, CurItem);
        }
      }
      /* Move the item */
      TV_INSERTSTRUCT Insert;
      Insert.item = TVItem;
      Insert.hInsertAfter = PrevItem;
      Insert.hParent = Parent;
      HTREEITEM NewItem = TreeView_InsertItem(Handle, &Insert);
      /* Move child items */
      HTREEITEM ChildItem = TreeView_GetChild(Handle, Item);
      while (ChildItem != NULL)
      {
        HTREEITEM NextItem = TreeView_GetNextSibling(Handle, ChildItem);
        if (GetTreeViewItemIcon(ChildItem) > 0)
          MoveTreeViewFolder(NewItem, ChildItem);
        else
          MoveTreeViewFile(NewItem, ChildItem);
        ChildItem = NextItem;
      }
      /* Expands parent */
      TreeView_Expand(Handle, Parent, TVE_EXPAND);
      /* Select new item */
      SelectTreeViewItem(NewItem, TVGN_CARET);
    }
    TreeView_DeleteItem(Handle, Item);
    delete[] Caption;
  }
}

void ProjectExplorer::SaveFolderToFile(ofstream& File, HTREEITEM Parent, string FolderName)
{
  if (Parent == TreeView_GetRoot(Handle))
  {
    FolderName = GetTreeViewItemName(Parent);
    File.write("Project=", 8);
    File.write(FolderName.c_str(), FolderName.size());
    FolderName = "";
  }
  else
  {
    if (FolderName.size() > 0)
      FolderName.append(1, '\\');
    FolderName.append(GetTreeViewItemName(Parent));
    File.write("Folder=", 7);
    File.write(FolderName.c_str(), FolderName.size());
  }
  File.write("\n", 1);
  /* Save files */
  HTREEITEM Item = TreeView_GetChild(Handle, Parent);
  while (Item != NULL)
  {
    if (GetTreeViewItemIcon(Item) == 0)
    {
      const char* Str = ((string*)GetTreeViewItemLParam(Item))->c_str();
      File.write("File=", 5);
      if (SendMessage(GetParent(Handle), WM_FILEOPENED, (WPARAM)Str, 0) > 0)
        File.write("Opened;", 7);
      else
        File.write("Closed;", 7);
      File.write(Str, strlen(Str));
      File.write("\n", 1);
    }
    Item = TreeView_GetNextSibling(Handle, Item);
  }
  /* Save sub-folder */
  Item = TreeView_GetChild(Handle, Parent);
  while (Item != NULL)
  {
    if (GetTreeViewItemIcon(Item) > 0)
      SaveFolderToFile(File, Item, FolderName);
    else
      return;
    Item = TreeView_GetNextSibling(Handle, Item);
  }
}

void ProjectExplorer::SelectTreeViewItem(HTREEITEM Item, int Flag)
{
  if (Item != NULL)
  {
    if (Flag == TVGN_CARET && Item != TreeView_GetDropHilight(Handle))
      TreeView_Select(Handle, NULL, TVGN_DROPHILITE);
    TreeView_Select(Handle, Item, Flag);
    if (Flag == TVGN_CARET && GetTreeViewItemIcon(Item) == 0 && SingleClickOpen)
      SendMessage(GetParent(Handle), WM_COMMAND, MAKEWPARAM(ID_FILE_OPENFILE, 0), 0);
  }
}

void ProjectExplorer::SetTreeViewItemIcon(HTREEITEM Item, int Icon)
{
  if (Item != NULL)
  {
    TV_ITEM TVItem;
    TVItem.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    TVItem.hItem = Item;
    TVItem.iImage = Icon;
    TVItem.iSelectedImage = Icon;
    TreeView_SetItem(Handle, &TVItem);
  }
}

void ProjectExplorer::SetTreeViewItemName(HTREEITEM Item, string Name)
{
  if (Item != NULL)
  {
    char* Caption = new char [MAX_PATH];
    strcpy(Caption, Name.c_str());
    TV_ITEM TVItem;
    TVItem.mask = TVIF_TEXT;
    TVItem.hItem = Item;
    TVItem.pszText = Caption;
    TreeView_SetItem(Handle, &TVItem);
    delete[] Caption;
  }
}

void ProjectExplorer::SetTreeViewItemLParam(HTREEITEM Item, int lParam)
{
  if (Item != NULL)
  {
    TV_ITEM TVItem;
    TVItem.mask = TVIF_PARAM;
    TVItem.hItem = Item;
    TVItem.lParam = lParam;
    TreeView_SetItem(Handle, &TVItem);
  }
}

// WINAPI FUNCTIONS ------------------------------------------------------------

HWND ProjectExplorer::CreateWnd(HWND hParent)
{
  HINSTANCE hInstance = GetModuleHandle(NULL);
  /* Creates the tree view */
  HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES, WC_TREEVIEW, NULL,
    WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP
    | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS,
    0, 0, 0, 0, hParent, NULL, hInstance, NULL);
  SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
  DefaultWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WindowProc);
  /* Create the image list */
  HIMAGELIST hList = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_COLOR16 | ILC_MASK, 0, 1);
  if (hList == NULL)
      return 0;
  HBITMAP Bitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(ID_EXPLORERICONS));
  ImageList_AddMasked(hList, Bitmap, RGB(255, 0, 255));
  DeleteObject(Bitmap);
  /* Associate the image list with the tree-view control */
  TreeView_SetImageList(hWnd, hList, TVSIL_NORMAL);
  /* Set auto-hide counter */
  if (AutoHide)
    SetTimer(GetParent(hWnd), 1, AutoHideDelay, NULL);
  return hWnd;
}

LRESULT CALLBACK ProjectExplorer::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static bool Dragging = false;
  static POINT Pos;
  ProjectExplorer* Explorer = (ProjectExplorer*)GetWindowLong(hWnd, GWL_USERDATA);
  switch (Msg)
  {
    case WM_CHAR:
    {
      if (wParam == 13 || wParam == 127)
        return 0;
      else
        break;
    }
    case WM_COMMAND:
    {
      /* Edit item */
      if (HIWORD(wParam) == EN_KILLFOCUS)
        Explorer->SetTreeViewItemName(TreeView_GetSelection(hWnd), GetWindowText((HWND)lParam));
      break;
    }
    case WM_DROPFILES:
    {
      int Count = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, 0, 0);
      for (int i = 0; i < Count; i++)
      {
        char FileName[MAX_PATH];
        DragQueryFile((HDROP)wParam, i, FileName, MAX_PATH);
        if (IsDirectory(FileName))
          Explorer->AddDirectory("", FileName);
        else
          Explorer->AddFile("", FileName);
      }
      DragFinish((HDROP)wParam);
      return 0;
    }
    case WM_GETDLGCODE:
    {
      return DLGC_WANTCHARS;
    }
    case WM_KEYUP:
    {
      switch (wParam)
      {
        case VK_F2:
        {
          SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(ID_EDIT_RENAMEPROJECTFILE, 0), 0);
          break;
        }
        case VK_DELETE:
        {
          SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(ID_FILE_REMOVEFROMPROJECT, 0), 0);
          break;
        }
        case VK_RETURN:
        {
          SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(ID_FILE_OPENFILE, 0), 0);
          break;
        }
      }
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      if (GetFocus() != hWnd)
        SetFocus(hWnd);
      if (GetCapture() != hWnd)
        SetCapture(hWnd);
      /* Select item */
      HTREEITEM Item = Explorer->GetTreeViewItem((short)LOWORD(lParam), (short)HIWORD(lParam));
      Explorer->SelectTreeViewItem(Item, TVGN_DROPHILITE);
      Pos.x = LOWORD(lParam);
      Pos.y = HIWORD(lParam);
      return 0;
    }
    case WM_LBUTTONUP:
    {
      if (Dragging)
      {
        /* End dragging item */
        ImageList_DragLeave(hWnd);
        ImageList_EndDrag();
        Dragging = false;
			  /* Drop item */
			  HTREEITEM Target = TreeView_GetDropHilight(hWnd);
			  HTREEITEM Item = TreeView_GetSelection(hWnd);
			  if (Explorer->GetTreeViewItemIcon(Target) > 0 && !Explorer->IsChildOf(Item, Target))
			  {
          if (Explorer->GetTreeViewItemIcon(Item) > 0)
            Explorer->MoveTreeViewFolder(Target, Item);
          else
            Explorer->MoveTreeViewFile(Target, Item);
        }
      }
      else
      {
        if (GetFocus() != hWnd)
          SetFocus(hWnd);
        /* Determine which area was clicked */
        RECT ItemRect;
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        HTREEITEM Item = Explorer->GetTreeViewItem(x, y);
        TreeView_GetItemRect(hWnd, Item, &ItemRect, TRUE);
        /* Toggle, select or rename item */
        if (x < ItemRect.left-ICON_SIZE-4 && x > ItemRect.left-ICON_SIZE-24 && y > ItemRect.top && y < ItemRect.bottom)
          TreeView_Expand(hWnd, Item, TVE_TOGGLE);
        else if (Item != TreeView_GetSelection(hWnd))
          Explorer->SelectTreeViewItem(Item, TVGN_CARET);
        else if (Explorer->RenameOnClick)
          Explorer->EditSelected();
      }
      ReleaseCapture();
      return 0;
    }
    case WM_RBUTTONUP:
    {
      ReleaseCapture();
      /* Select item */
      if (GetFocus() != hWnd)
        SetFocus(hWnd);
      HTREEITEM Item = Explorer->GetTreeViewItem((short)LOWORD(lParam), (short)HIWORD(lParam));
      Explorer->SelectTreeViewItem(Item, TVGN_CARET);
      /* Show context menu */
      POINT p;
      if (GetCursorPos(&p))
        SendMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)hWnd, MAKELPARAM(p.x, p.y));
      return 0;
    }
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
      /* Open selected item */
      if (!Explorer->SingleClickOpen)
        SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(ID_FILE_OPENFILE, 0), 0);
      return 0;
    }
    case WM_MOUSEMOVE:
    {
      if (Dragging)
      {
        /* Move dragged item */
        ImageList_DragMove((short)LOWORD(lParam)-6, (short)HIWORD(lParam)-6);
				ImageList_DragShowNolock(FALSE);
        HTREEITEM Item = Explorer->GetTreeViewItem((short)LOWORD(lParam), (short)HIWORD(lParam));
        Explorer->SelectTreeViewItem(Item, TVGN_DROPHILITE);
			  ImageList_DragShowNolock(TRUE);
			  /* Change cursor */
			  if (Explorer->GetTreeViewItemIcon(Item) > 0 && !Explorer->IsChildOf(TreeView_GetSelection(hWnd), Item))
  			  SetCursor(LoadCursor(NULL, IDC_ARROW));
        else
	 		    SetCursor(LoadCursor(NULL, IDC_NO));
      }
      else if (wParam & MK_LBUTTON && (abs(LOWORD(lParam)-Pos.x) > 5 || abs(HIWORD(lParam)-Pos.y) > 5))
      {
        HTREEITEM Item = Explorer->GetTreeViewItem((short)LOWORD(lParam), (short)HIWORD(lParam));
        if (Item != NULL)
        {
          /* Begin dragging item */
          Explorer->SelectTreeViewItem(Item, TVGN_CARET);
          ImageList_BeginDrag(TreeView_CreateDragImage(hWnd, Item), 0, 0, 0);
          ImageList_DragEnter(hWnd, (short)LOWORD(lParam)-6, (short)HIWORD(lParam)-6);
          Dragging = true;
        }
      }
      /* Reset auto-hide counter */
      if (Explorer->AutoHide)
        SetTimer(GetParent(hWnd), 1, Explorer->AutoHideDelay, NULL);
      return 0;
    }
    case WM_NCPAINT:
    {
      /* Call de default window proc */
      CallWindowProc((WNDPROC)Explorer->DefaultWndProc, hWnd, Msg, wParam, lParam);
      /* Paints the window border */
      RECT R;
      GetWindowRect(hWnd, &R);
      R.right -= R.left;
      R.bottom -= R.top;
      R.left = 0;
      R.top = 0;
      HDC DC = GetWindowDC(hWnd);
      DrawEdge(DC, &R, EDGE_SUNKEN, BF_RECT);
      ReleaseDC(hWnd, DC);
      return 0;
    }
    case WM_TIMER:
    {
      if (wParam == 1)
        Explorer->SetHide(true);
      KillTimer(hWnd, wParam);
      return 0;
    }
  }
  return CallWindowProc((WNDPROC)Explorer->DefaultWndProc, hWnd, Msg, wParam, lParam);
}
