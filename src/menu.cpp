#include "main.h"
#include "resource.h"
#include "models/stringlist.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/inputdialog.h"
#include "dialogs/finddialog.h"
#include "dialogs/gotodialog.h"
#include "dialogs/datetimedialog.h"
#include "dialogs/preferencesdialog.h"
#include "dialogs/propertiesdialog.h"

TStringList Templates;

COLORREF LastColor = 0;
COLORREF CustomColors[16];

// PRIVATE FUNCTIONS -----------------------------------------------------------

void AppendMenu(HMENU hMenu, UINT Id)
{
  char Str[MAX_PATH];
  LoadString(hInstance, Id, Str, MAX_PATH);
  AppendMenu(hMenu, MF_STRING, Id, Str);
}

void AppendMenu(HMENU hMenu, UINT Id, HMENU hSubMenu)
{
  char Str[MAX_PATH];
  LoadString(hInstance, Id, Str, MAX_PATH);
  AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hSubMenu, Str);
}

// PUBLIC FUNCTIONS ------------------------------------------------------------

HMENU FileNewMenu()
{
  static HMENU hFileNewMenu = NULL;
  if (hFileNewMenu == NULL)
  {
    hFileNewMenu = CreatePopupMenu();
    AppendMenu(hFileNewMenu, ID_FILE_NEW_PROJECT);
    AppendMenu(hFileNewMenu, ID_FILE_NEW_EMPTY);
    AppendMenu(hFileNewMenu, MF_SEPARATOR, 0, NULL);
  }
  return hFileNewMenu;
}

HMENU FileMenu()
{
  static HMENU hFileMenu = NULL;
  if (hFileMenu == NULL)
  {
    hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, ID_FILE_NEW, FileNewMenu());
    AppendMenu(hFileMenu, ID_FILE_OPEN);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_SAVE);
    AppendMenu(hFileMenu, ID_FILE_SAVEAS);
    AppendMenu(hFileMenu, ID_FILE_SAVEALL);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_SAVEPROJ);
    AppendMenu(hFileMenu, ID_FILE_SAVETEMPLATE);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_CLOSEDOC);
    AppendMenu(hFileMenu, ID_FILE_CLOSEALL);
    AppendMenu(hFileMenu, ID_FILE_CLOSEOTHERS);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_PRINT);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_ADDPROJECTFILE);
    AppendMenu(hFileMenu, ID_FILE_REMOVEPROJECTFILE);
    AppendMenu(hFileMenu, ID_FILE_ADDPROJECTFOLDER);
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, ID_FILE_EXIT);
  }
  return hFileMenu;
}

HMENU EditGotoBookmarkMenu()
{
  static HMENU hEditGotoBookmarkMenu = NULL;
  if (hEditGotoBookmarkMenu == NULL)
  {
    hEditGotoBookmarkMenu = CreatePopupMenu();
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_1);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_2);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_3);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_4);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_5);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_6);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_7);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_8);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_9);
    AppendMenu(hEditGotoBookmarkMenu, ID_EDIT_GOTOBOOKMARK_0);
  }
  return hEditGotoBookmarkMenu;
}

HMENU EditToggleBookmarkMenu()
{
  static HMENU hEditToggleBookmarkMenu = NULL;
  if (hEditToggleBookmarkMenu == NULL)
  {
    hEditToggleBookmarkMenu = CreatePopupMenu();
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_1);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_2);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_3);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_4);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_5);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_6);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_7);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_8);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_9);
    AppendMenu(hEditToggleBookmarkMenu, ID_EDIT_TOGGLEBOOKMARK_0);
  }
  return hEditToggleBookmarkMenu;
}

HMENU EditMenu()
{
  static HMENU hEditMenu = NULL;
  if (hEditMenu == NULL)
  {
    hEditMenu = CreatePopupMenu();
    AppendMenu(hEditMenu, ID_EDIT_UNDO);
    AppendMenu(hEditMenu, ID_EDIT_REDO);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_CUT);
    AppendMenu(hEditMenu, ID_EDIT_COPY);
    AppendMenu(hEditMenu, ID_EDIT_PASTE);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_SELECTALL);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_FIND);
    AppendMenu(hEditMenu, ID_EDIT_FINDNEXT);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_INDENT);
    AppendMenu(hEditMenu, ID_EDIT_UNINDENT);
    AppendMenu(hEditMenu, ID_EDIT_FORMAT);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_TOGGLEBOOKMARK, EditToggleBookmarkMenu());
    AppendMenu(hEditMenu, ID_EDIT_GOTOBOOKMARK, EditGotoBookmarkMenu());
    AppendMenu(hEditMenu, ID_EDIT_GOTOLINE);
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, ID_EDIT_PREFERENCES);
  }
  return hEditMenu;
}

HMENU ToolInsertCharMenu()
{
  static HMENU hToolInsertCharMenu = NULL;
  if (hToolInsertCharMenu == NULL)
  {
    hToolInsertCharMenu = CreatePopupMenu();
    for (int i = 0; i < CharactersCount; i++)
    {
      char* Str = new char[32];
      if (strcmp(Characters[i], "&") == 0)
        strcpy(Str, "&&");
      else if (strcmp(Characters[i], " ") == 0)
        strcpy(Str, "space");
      else
        strcpy(Str, Characters[i]);
      strcat(Str, " (&");
      strcat(Str, CharCodes[i]);
      strcat(Str, ")");
      AppendMenu(hToolInsertCharMenu, MF_STRING | (i%26 == 0 ? MF_MENUBREAK : 0), ID_TOOL_INSERTCHAR_ITEM+i, Str);
    }
  }
  return hToolInsertCharMenu;
}

HMENU ToolInsertSingleTagMenu()
{
  static HMENU hToolInsertSingleTagMenu = NULL;
  if (hToolInsertSingleTagMenu == NULL)
  {
    hToolInsertSingleTagMenu = CreatePopupMenu();
    for (int i = 0; i < SingleTagsCount; i++)
    {
      char* Str = new char[32];
      strcpy(Str, "<");
      strcat(Str, SingleTags[i]);
      strcat(Str, ">");
      AppendMenu(hToolInsertSingleTagMenu, MF_STRING, ID_TOOL_INSERTSINGLETAG_ITEM+i, Str);
    }
  }
  return hToolInsertSingleTagMenu;
}

HMENU ToolInsertPairedTagMenu()
{
  static HMENU hToolInsertPairedTagMenu = NULL;
  if (hToolInsertPairedTagMenu == NULL)
  {
    hToolInsertPairedTagMenu = CreatePopupMenu();
    for (int i = 0; i < PairedTagsCount; i++)
    {
      char* Str = new char[32];
      strcpy(Str, "<");
      strcat(Str, PairedTags[i]);
      strcat(Str, ">");
      strcat(Str, "</");
      strcat(Str, PairedTags[i]);
      strcat(Str, ">");
      AppendMenu(hToolInsertPairedTagMenu, MF_STRING | (i%24 == 0 ? MF_MENUBREAK : 0), ID_TOOL_INSERTPAIREDTAG_ITEM+i, Str);
    }
  }
  return hToolInsertPairedTagMenu;
}

HMENU ToolMenu()
{
  static HMENU hToolMenu = NULL;
  if (hToolMenu == NULL)
  {
    hToolMenu = CreatePopupMenu();
    AppendMenu(hToolMenu, ID_TOOL_BROWSEDOC);
    AppendMenu(hToolMenu, ID_TOOL_VALIDATEDOC);
    AppendMenu(hToolMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hToolMenu, ID_TOOL_INSERTCHAR, ToolInsertCharMenu());
    AppendMenu(hToolMenu, ID_TOOL_INSERTSINGLETAG, ToolInsertSingleTagMenu());
    AppendMenu(hToolMenu, ID_TOOL_INSERTPAIREDTAG, ToolInsertPairedTagMenu());
    AppendMenu(hToolMenu, ID_TOOL_INSERTCOLOR);
    AppendMenu(hToolMenu, ID_TOOL_INSERTDATETIME);
    AppendMenu(hToolMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hToolMenu, ID_TOOL_CHARTOHTML);
    AppendMenu(hToolMenu, ID_TOOL_HTMLTOCHAR);
    AppendMenu(hToolMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hToolMenu, ID_TOOL_AUTOCONVERT);
    AppendMenu(hToolMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hToolMenu, ID_TOOL_LOWERCASEHTML);
    AppendMenu(hToolMenu, ID_TOOL_UPPERCASEHTML);
  }
  return hToolMenu;
}

HMENU HelpMenu()
{
  static HMENU hHelpMenu = NULL;
  if (hHelpMenu == NULL)
  {
    hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu, ID_HELP_UPDATE);
    AppendMenu(hHelpMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hHelpMenu, ID_HELP_ABOUT);
  }
  return hHelpMenu;
}

HMENU MainMenu()
{
  static HMENU hMainMenu = NULL;
  if (hMainMenu == NULL)
  {
    hMainMenu = CreateMenu();
    AppendMenu(hMainMenu, ID_FILE, FileMenu());
    AppendMenu(hMainMenu, ID_EDIT, EditMenu());
    AppendMenu(hMainMenu, ID_TOOL, ToolMenu());
    AppendMenu(hMainMenu, ID_HELP, HelpMenu());
  }
  return hMainMenu;
}

HMENU EditorMenu()
{
  static HMENU hEditorMenu = NULL;
  if (hEditorMenu == NULL)
  {
    hEditorMenu = CreatePopupMenu();
    AppendMenu(hEditorMenu, ID_TOOL, ToolMenu());
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_UNDO);
    AppendMenu(hEditorMenu, ID_EDIT_REDO);
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_CUT);
    AppendMenu(hEditorMenu, ID_EDIT_COPY);
    AppendMenu(hEditorMenu, ID_EDIT_PASTE);
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_SELECTALL);
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_FIND);
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_INDENT);
    AppendMenu(hEditorMenu, ID_EDIT_UNINDENT);
    AppendMenu(hEditorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditorMenu, ID_EDIT_GOTOBOOKMARK, EditGotoBookmarkMenu());
    AppendMenu(hEditorMenu, ID_EDIT_TOGGLEBOOKMARK, EditToggleBookmarkMenu());
  }
  /* Update items */
  if (Editor->GetDocument()->IsSelected())
  {
    EnableMenuItem(hEditorMenu, ID_EDIT_CUT, MF_ENABLED);
    EnableMenuItem(hEditorMenu, ID_EDIT_COPY, MF_ENABLED);
  }
  else
  {
    EnableMenuItem(hEditorMenu, ID_EDIT_CUT, MF_GRAYED);
    EnableMenuItem(hEditorMenu, ID_EDIT_COPY, MF_GRAYED);
  }
  return hEditorMenu;
}

HMENU ExplorerMenu()
{
  static HMENU hExplorerMenu = NULL;
  if (hExplorerMenu == NULL)
  {
    hExplorerMenu = CreatePopupMenu();
    AppendMenu(hExplorerMenu, ID_FILE_OPENFILE);
    AppendMenu(hExplorerMenu, ID_FILE_CLOSEFILE);
    AppendMenu(hExplorerMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hExplorerMenu, ID_FILE_ADDPROJECTFILE);
    AppendMenu(hExplorerMenu, ID_FILE_ADDPROJECTFOLDER);
    AppendMenu(hExplorerMenu, ID_FILE_REMOVEFROMPROJECT);
    AppendMenu(hExplorerMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hExplorerMenu, ID_EDIT_RENAMEPROJECTFILE);
    AppendMenu(hExplorerMenu, ID_EDIT_FILEPROPERTIES);
    AppendMenu(hExplorerMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hExplorerMenu, ID_TOOL_BROWSEFILE);
    AppendMenu(hExplorerMenu, ID_TOOL_VALIDATEFILE);
  }
  /* Update items */
  if (Explorer->GetSelectedFile().size() > 0)         // A file is selected
  {
    EnableMenuItem(hExplorerMenu, ID_FILE_REMOVEFROMPROJECT, MF_ENABLED);
    if (Editor->DocumentIndex(Explorer->GetSelectedFile()) >= 0)    // Is opened
    {
      EnableMenuItem(hExplorerMenu, ID_FILE_OPENFILE, MF_GRAYED);
      EnableMenuItem(hExplorerMenu, ID_FILE_CLOSEFILE, MF_ENABLED);
    }
    else
    {
      EnableMenuItem(hExplorerMenu, ID_FILE_OPENFILE, MF_ENABLED);
      EnableMenuItem(hExplorerMenu, ID_FILE_CLOSEFILE, MF_GRAYED);
    }
    EnableMenuItem(hExplorerMenu, ID_EDIT_RENAMEPROJECTFILE, MF_ENABLED);
    EnableMenuItem(hExplorerMenu, ID_EDIT_FILEPROPERTIES, MF_ENABLED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_BROWSEFILE, MF_ENABLED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_VALIDATEFILE, MF_ENABLED);
  }
  else if (Explorer->GetSelectedFolder().size() > 0)  // A folder is selected
  {
    EnableMenuItem(hExplorerMenu, ID_FILE_REMOVEFROMPROJECT, MF_ENABLED);
    EnableMenuItem(hExplorerMenu, ID_FILE_OPENFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_FILE_CLOSEFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_EDIT_RENAMEPROJECTFILE, MF_ENABLED);
    EnableMenuItem(hExplorerMenu, ID_EDIT_FILEPROPERTIES, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_BROWSEFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_VALIDATEFILE, MF_GRAYED);
  }
  else
  {
    EnableMenuItem(hExplorerMenu, ID_FILE_REMOVEFROMPROJECT, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_FILE_OPENFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_FILE_CLOSEFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_EDIT_RENAMEPROJECTFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_EDIT_FILEPROPERTIES, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_BROWSEFILE, MF_GRAYED);
    EnableMenuItem(hExplorerMenu, ID_TOOL_VALIDATEFILE, MF_GRAYED);
  }
  return hExplorerMenu;
}

HMENU TabsMenu()
{
  static HMENU hTabsMenu = NULL;
  if (hTabsMenu == NULL)
  {
    hTabsMenu = CreatePopupMenu();
    AppendMenu(hTabsMenu, ID_FILE_CLOSEDOC);
    AppendMenu(hTabsMenu, ID_FILE_CLOSEALL);
    AppendMenu(hTabsMenu, ID_FILE_CLOSEOTHERS);
    AppendMenu(hTabsMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hTabsMenu, ID_FILE_ADDTOPROJECT);
    AppendMenu(hTabsMenu, ID_EDIT_RENAMEDOCUMENT);
    AppendMenu(hTabsMenu, ID_EDIT_DOCPROPERTIES);
    AppendMenu(hTabsMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hTabsMenu, ID_TOOL_BROWSEDOC);
    AppendMenu(hTabsMenu, ID_TOOL_VALIDATEDOC);
  }
  /* Update items */
  if (Editor->DocumentCount() > 1)
  {
    EnableMenuItem(hTabsMenu, ID_FILE_CLOSEOTHERS, MF_ENABLED);
    EnableMenuItem(hTabsMenu, ID_FILE_CLOSEALL, MF_ENABLED);
  }
  else
  {
    EnableMenuItem(hTabsMenu, ID_FILE_CLOSEOTHERS, MF_GRAYED);
    EnableMenuItem(hTabsMenu, ID_FILE_CLOSEALL, MF_GRAYED);
  }
  return hTabsMenu;
}

void UpdateMenuTemplates()
{
  /* Delete template menu items */
  while (GetMenuItemCount(FileNewMenu()) > 3)
    DeleteMenu(FileNewMenu(), 3, MF_BYPOSITION);
  /* Find files in the templates folder and add them to menu */
  WIN32_FIND_DATA FindData;
  char* Path = new char[MAX_PATH];
  char* File = new char[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, Path);
  strcat(Path, "\\templates\\");
  strcpy(File, Path);
  strcat(File, "*.*");
  HANDLE Handle = FindFirstFile(File, &FindData);
  if (Handle != INVALID_HANDLE_VALUE)
  {
    int i = 0;
    Templates.Clear();
    do if (FindData.cFileName[0] != '.' && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      Templates.Add(Path);
      Templates[i].append(FindData.cFileName);
      int Index = Templates[i].rfind('\\')+1;
      int Length = Templates[i].rfind('.')-Index;
      AppendMenu(FileNewMenu(), MF_STRING, ID_FILE_NEW_TEMP+i, Templates[i].substr(Index, Length).c_str());
      i++;
    }
    while (FindNextFile(Handle, &FindData) && i < ID_EDIT-ID_FILE_NEW_TEMP);
    FindClose(Handle);
  }
  delete[] Path;
  delete[] File;
}

// WINAPI FUNCTIONS ------------------------------------------------------------

void __stdcall MenuProc(WORD ID)
{
  switch (ID)
  {
    case ID_FILE_NEW:
    {
      /* Update the list of templates */
      UpdateMenuTemplates();
      /* Exists only for the toolbar */
      POINT P;
      GetCursorPos(&P);
      TrackPopupMenu(FileNewMenu(), TPM_LEFTALIGN, P.x, P.y, 0, hWindow, NULL);
      break;
    }
    case ID_FILE_NEW_PROJECT:
    {
      string Str = InputDialog("New project", "Enter the name of the new project:", "");
      if (Str != "")
      {
        Explorer->Clear();
        Explorer->SetProjectName(Str);
      }
      break;
    }
    case ID_FILE_NEW_EMPTY:
    {
      Editor->NewDocument();
      break;
    }
    case ID_FILE_OPEN:
    {
      Open();
      break;
    }
    case ID_FILE_SAVE:
    {
      SaveDocument(Editor->DocumentIndex());
      break;
    }
    case ID_FILE_SAVEAS:
    {
      SaveDocumentAs(Editor->DocumentIndex());
      break;
    }
    case ID_FILE_SAVEALL:
    {
      SaveAll();
      break;
    }
    case ID_FILE_SAVEPROJ:
    {
      SaveProjectAs();
      break;
    }
    case ID_FILE_SAVETEMPLATE:
    {
      SaveAsTemplate(Editor->DocumentIndex());
      break;
    }
    case ID_FILE_ADDPROJECTFILE:
    {
      AddToProject();
      break;
    }
    case ID_FILE_ADDPROJECTFOLDER:
    {
      string Str = InputDialog("New folder", "Enter the name of the new folder:", "");
      if (Str != "")
        Explorer->AddFolder(Explorer->GetSelectedFolder(), Str);
      UpdateToolbar();
      break;
    }
    case ID_FILE_REMOVEPROJECTFILE:
    {
      // opens a dialog
      UpdateToolbar();
      break;
    }
    case ID_FILE_REMOVEFROMPROJECT:
    {
      Explorer->DeleteSelected();
      UpdateToolbar();
      break;
    }
    case ID_FILE_CLOSEFILE:
    {
      string Name = Explorer->GetSelectedFile();
      if (Name.size() > 0)
      {
        Editor->CloseDocument(Editor->DocumentIndex(Name));
        if (Editor->DocumentCount() < 1)
          Editor->NewDocument();
      }
      break;
    }
    case ID_FILE_ADDTOPROJECT:
    {
      if (Editor->GetDocument() != NULL)
      {
        string Name = Editor->GetDocument()->GetFileName();
        if (Name.size() == 0)
          SaveDocument(Editor->DocumentIndex());
        if (Name.size() > 0)
          Explorer->AddFile(Explorer->GetSelectedFolder(), Name);
        UpdateToolbar();
      }
      break;
    }
    case ID_FILE_CLOSEDOC:
    {
      Editor->CloseDocument(Editor->DocumentIndex());
      if (Editor->DocumentCount() < 1)
        Editor->NewDocument();
      break;
    }
    case ID_FILE_CLOSEALL:
    {
      while (Editor->DocumentCount() > 0)
        Editor->CloseDocument(0);
      Editor->NewDocument();
      break;
    }
    case ID_FILE_CLOSEOTHERS:
    {
      int DocCount = Editor->DocumentCount();
      int DocIndex = Editor->DocumentIndex();
      for (int i = 0; i < DocCount-1; i++)
        Editor->CloseDocument(i >= DocIndex ? 1 : 0);
      break;
    }
    case ID_FILE_PRINT:
    {
      Editor->Print(PrintDialog(hWindow));
      break;
    }
    case ID_FILE_EXIT:
    {
      Quit();
      break;
    }
    case ID_EDIT_UNDO:
    {
      if (Editor->GetDocument() != NULL)
        Editor->GetDocument()->Undo(Editor->GroupUndo);
      break;
    }
    case ID_EDIT_REDO:
    {
      if (Editor->GetDocument() != NULL)
        Editor->GetDocument()->Redo(Editor->GroupUndo);
      break;
    }
    case ID_EDIT_CUT:
    {
      Editor->CutToClipboard();
      break;
    }
    case ID_EDIT_COPY:
    {
      Editor->CopyToClipboard();
      break;
    }
    case ID_EDIT_PASTE:
    {
      Editor->PasteFromClipboard();
      break;
    }
    case ID_EDIT_SELECTALL:
    {
      Editor->SelectAll();
      break;
    }
    case ID_EDIT_FIND:
    {
      ShowFindDialog();
      break;
    }
    case ID_EDIT_FINDNEXT:
    {
      FindNext();
      break;
    }
    case ID_EDIT_INDENT:
    {
      if (Editor->GetDocument() != NULL)
        Editor->GetDocument()->IndentLines(Editor->GetDocument()->GetSelStart().y, Editor->GetDocument()->GetSelEnd().y, Editor->TabSize);
      break;
    }
    case ID_EDIT_UNINDENT:
    {
      if (Editor->GetDocument() != NULL)
        Editor->GetDocument()->UnindentLines(Editor->GetDocument()->GetSelStart().y, Editor->GetDocument()->GetSelEnd().y, Editor->TabSize);
      break;
    }
    case ID_EDIT_FORMAT:
    {
      if (Editor->GetDocument() != NULL)
        if (PointCmp(Editor->GetDocument()->GetSelStart(), Editor->GetDocument()->GetSelEnd()) != 0)
          Editor->GetDocument()->FormatLines(Editor->GetDocument()->GetSelStart().y, Editor->GetDocument()->GetSelEnd().y, Editor->GetMarginSize());
        else
          Editor->GetDocument()->FormatLines(0, Editor->GetDocument()->GetLineCount()-1, Editor->GetMarginSize());
      break;
    }
    case ID_EDIT_GOTOBOOKMARK_1:
    case ID_EDIT_GOTOBOOKMARK_2:
    case ID_EDIT_GOTOBOOKMARK_3:
    case ID_EDIT_GOTOBOOKMARK_4:
    case ID_EDIT_GOTOBOOKMARK_5:
    case ID_EDIT_GOTOBOOKMARK_6:
    case ID_EDIT_GOTOBOOKMARK_7:
    case ID_EDIT_GOTOBOOKMARK_8:
    case ID_EDIT_GOTOBOOKMARK_9:
    case ID_EDIT_GOTOBOOKMARK_0:
    {
      Editor->GotoBookmark(ID-ID_EDIT_GOTOBOOKMARK_1);
      break;
    }
    case ID_EDIT_TOGGLEBOOKMARK_1:
    case ID_EDIT_TOGGLEBOOKMARK_2:
    case ID_EDIT_TOGGLEBOOKMARK_3:
    case ID_EDIT_TOGGLEBOOKMARK_4:
    case ID_EDIT_TOGGLEBOOKMARK_5:
    case ID_EDIT_TOGGLEBOOKMARK_6:
    case ID_EDIT_TOGGLEBOOKMARK_7:
    case ID_EDIT_TOGGLEBOOKMARK_8:
    case ID_EDIT_TOGGLEBOOKMARK_9:
    case ID_EDIT_TOGGLEBOOKMARK_0:
    {
      Editor->ToggleBookmark(ID-ID_EDIT_TOGGLEBOOKMARK_1);
      break;
    }
    case ID_EDIT_GOTOLINE:
    {
      ShowGotoDialog();
      SetFocus(Editor->GetHandle());
      break;
    }
    case ID_FILE_OPENFILE:
    {
      string Name = Explorer->GetSelectedFile();
      if (Name.size() > 0)
        OpenDocument(Name.c_str());
      else
        MessageBox(hWindow, "The selected file cannot be found. It may have been moved or deleted", "Open file", MB_OK);
      break;
    }
    case ID_EDIT_RENAMEDOCUMENT:
    {
      if (Editor->GetDocument() != NULL)
      {
        string Name = Editor->GetDocument()->GetFileName();
        string Str = InputDialog("Rename document", "Enter the new document name:", Name);
        if (Str != "")
        {
          rename(Name.c_str(), Str.c_str());          // Renames the actual file
          Editor->GetDocument()->SetFileName(Str);
          Explorer->RenameFile(Name, Str);
        }
      }
      break;
    }
    case ID_EDIT_RENAMEPROJECTFILE:
    {
      string Name = Explorer->GetSelectedFile();
      if (Name != "")
      {
        string Str = InputDialog("Rename file", "Enter the new file name:", Name);
        if (Str != "")
        {
          rename(Name.c_str(), Str.c_str());        // Renames the actual file
          if (Editor->GetDocument(Editor->DocumentIndex(Name)) != NULL)
          {
            Editor->GetDocument(Editor->DocumentIndex(Name))->SetFileName(Str);
            Explorer->RenameFile(Name, Str);
          }
        }
      }
      else
        Explorer->EditSelected();
      break;
    }
    case ID_EDIT_DOCPROPERTIES:
    {
      if (Editor->GetDocument() != NULL)
      {
        string Name = Editor->GetDocument()->GetFileName();
        if (Name.size() > 0)
          ShowPropertiesDialog(Name);
      }
      break;
    }
    case ID_EDIT_FILEPROPERTIES:
    {
      string Name = Explorer->GetSelectedFile();
      if (Name.size() > 0)
        ShowPropertiesDialog(Name);
      break;
    }
    case ID_EDIT_OVERWRITEMODE:
    {
      Editor->OverwriteMode = !Editor->OverwriteMode;
      UpdateStatusbar();
      break;
    }
    case ID_TOOL_INSERTCOLOR:
    {
      if (Editor->GetDocument() != NULL)
      {
        TPoint Start = Editor->GetDocument()->GetSelStart();
        TPoint End = Editor->GetDocument()->GetSelEnd();
        COLORREF Color = LastColor;
        if (Start.y == End.y)
          Color = HTMLToColor(Editor->GetDocument()->GetString(Start, End).c_str());
        CHOOSECOLOR ColorDialog;
        ColorDialog.lStructSize = sizeof(CHOOSECOLOR);
        ColorDialog.hwndOwner = hWindow;
        ColorDialog.rgbResult = Color;
        ColorDialog.lpCustColors = &CustomColors[0];
        ColorDialog.Flags = CC_FULLOPEN | CC_RGBINIT;
        ColorDialog.lCustData = 0;
        if (ChooseColor(&ColorDialog) != 0)
        {
          if (Editor->OverwriteSelection)
            Editor->GetDocument()->DeleteString(Start, End);
          LastColor = ColorDialog.rgbResult;
          char* Str = ColorToHTML(LastColor);
          Editor->GetDocument()->InsertString(Editor->GetDocument()->GetCaretPos(), Str);
          delete[] Str;
        }
      }
      break;
    }
    case ID_TOOL_INSERTDATETIME:
    {
      if (Editor->GetDocument() != NULL)
      {
        char* Str = ShowDateTimeDialog(); // Do NOT delete, is a static array
        if (strlen(Str) > 0)
        {
          if (Editor->OverwriteSelection)
            Editor->GetDocument()->DeleteString(Editor->GetDocument()->GetSelStart(), Editor->GetDocument()->GetSelEnd());
          Editor->GetDocument()->InsertString(Editor->GetDocument()->GetCaretPos(), Str);
        }
      }
      break;
    }
    case ID_TOOL_CHARTOHTML:
    {
      for (int i = 5; i < CharactersCount; i++)
        ReplaceAll(Characters[i], CharCodes[i], false, true, false);
      break;
    }
    case ID_TOOL_HTMLTOCHAR:
    {
      for (int i = 5; i < CharactersCount; i++)
        ReplaceAll(CharCodes[i], Characters[i], false, true, false);
      break;
    }
    case ID_TOOL_AUTOCONVERT:
    {
      AutoConvertChars = !AutoConvertChars;
      CheckMenuItem(ToolMenu(), ID_TOOL_AUTOCONVERT, AutoConvertChars ? MF_CHECKED : MF_UNCHECKED);
      break;
    }
    case ID_TOOL_LOWERCASEHTML:
    {
      ChangeHTMLTagCase(false);
      break;
    }
    case ID_TOOL_UPPERCASEHTML:
    {
      ChangeHTMLTagCase(true);
      break;
    }
    case ID_TOOL_BROWSEDOC:
    {
      if (Editor->GetDocument() != NULL)
        OpenInBrowser(Editor->GetDocument()->GetFileName().c_str());
      break;
    }
    case ID_TOOL_BROWSEFILE:
    {
      OpenInBrowser(Explorer->GetSelectedFile().c_str());
      break;
    }
    case ID_TOOL_VALIDATEDOC:
    case ID_TOOL_VALIDATEFILE:
    {
      ShellExecute(hWindow, "open", "http://validator.w3.org/file-upload.html", NULL, NULL, SW_SHOWNORMAL);
      break;
    }
    case ID_EDIT_PREFERENCES:
    {
      ShowPreferencesDialog();
      SaveSettings();
      break;
    }
    case ID_HELP_UPDATE:
    {
      break;
    }
    case ID_HELP_ABOUT:
    {
      ShowAboutDialog();
      break;
    }
    case ID_EDIT_PROJECTEXPLORER:
    {
      if (Explorer->GetHide())
        Explorer->SetHide(false);
      SetFocus(Explorer->GetHandle());
      break;
    }
    default:
    {
      if (ID >= ID_FILE_NEW_TEMP && ID < ID_FILE_NEW_TEMP+Templates.Count())
      {
        Editor->LoadDocument(Templates[ID - ID_FILE_NEW_TEMP].c_str());
        if (Editor->GetDocument() != NULL)
          Editor->GetDocument()->SetFileName("");
      }
      else if (ID >= ID_TOOL_INSERTCHAR_ITEM && ID < ID_TOOL_INSERTCHAR_ITEM+200)
      {
        if (Editor->GetDocument() != NULL)
        {
          if (Editor->OverwriteSelection)
            Editor->GetDocument()->DeleteString(Editor->GetDocument()->GetSelStart(), Editor->GetDocument()->GetSelEnd());
          Editor->GetDocument()->InsertString(Editor->GetDocument()->GetCaretPos(), CharCodes[ID-ID_TOOL_INSERTCHAR_ITEM]);
        }
      }
      else if  (ID >= ID_TOOL_INSERTSINGLETAG_ITEM && ID < ID_TOOL_INSERTSINGLETAG_ITEM+100)
      {
        if (Editor->GetDocument() != NULL)
        {
          if (Editor->OverwriteSelection)
            Editor->GetDocument()->DeleteString(Editor->GetDocument()->GetSelStart(), Editor->GetDocument()->GetSelEnd());
          Editor->GetDocument()->DeleteString(Editor->GetDocument()->GetSelStart(), Editor->GetDocument()->GetSelEnd());
          string Str("<");
          Str.append(SingleTags[ID-ID_TOOL_INSERTSINGLETAG_ITEM]);
          Str.append(">");
          Editor->GetDocument()->InsertString(Editor->GetDocument()->GetCaretPos(), Str);
        }
      }
      else if  (ID >= ID_TOOL_INSERTPAIREDTAG_ITEM && ID < ID_TOOL_INSERTPAIREDTAG_ITEM+100)
      {
        if (Editor->GetDocument() != NULL)
        {
          // Do not delete selection, since we are surrounding it with the tag.
          TPoint Start = Editor->GetDocument()->GetSelStart();
          string Str("</");
          Str.append(PairedTags[ID-ID_TOOL_INSERTPAIREDTAG_ITEM]);
          Str.append(">");
          Editor->GetDocument()->InsertString(Editor->GetDocument()->GetSelEnd(), Str);
          Str.erase(1, 1);
          Editor->GetDocument()->InsertString(Start, Str);
        }
      }
    }
  }
}
