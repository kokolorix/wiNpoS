#pragma once
#include "WinPosWnd.h"

class HooksImpl
{
public:
	static LRESULT CALLBACK shellHookProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK callWndProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK getMsgProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);

private:
	WinPosWnd _winPosWnd;

private:
	void onNcLButtonDblClick(MSG* pMsg);
	void onWindowPosChanged(CWPSTRUCT* pMsg);
	
	enum struct IncWnd : unsigned char { Left = 0x01, Right = 0x02, Up = 0x04, Down = 0x08, All = Left|Right|Up|Down, Horizontal=Left|Right, Vertical=Up|Down };
	void onIncrementWindow(MSG* pMsg, int diff, IncWnd incDir = IncWnd::All, POINT* pCursorPos = nullptr, LRESULT hitTest = HTNOWHERE);
	void onNcLButtonDown(MSG* pMsg);
	void onLButtonUp(MSG* pMsg);
	void onShowPosWnd(MSG* pMsg, POINT pt);
};

