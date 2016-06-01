#ifndef SPLITTER_H
#define	SPLITTER_H

extern bool HideSplitter;
extern int SplitterPos;

LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HWND CreateSplitter(HWND Parent);

#endif
