#ifndef MENU_H
#define	MENU_H

#include <windows.h>

/* Sub menus */
HMENU FileNewMenu();
HMENU FileMenu();
HMENU EditGotoBookmarkMenu();
HMENU EditToggleBookmarkMenu();
HMENU EditMenu();
HMENU ToolInsertCharMenu();
HMENU ToolInsertSingleTagMenu();
HMENU ToolInsertPairedTagMenu();
HMENU ToolMenu();
HMENU HelpMenu();

/* Menus */
HMENU MainMenu();
HMENU EditorMenu();
HMENU ExplorerMenu();
HMENU TabsMenu();

void UpdateMenuTemplates();
void __stdcall MenuProc(WORD ID);

#endif

