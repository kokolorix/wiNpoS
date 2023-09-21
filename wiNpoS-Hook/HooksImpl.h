#pragma once

class HooksImpl
{
public:
	static LRESULT CALLBACK callWndProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK getMsgProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);

private:
	static void onNcLButtonDblClick(MSG* pMsg);
	static void onWindowPosChanged(CWPSTRUCT* pMsg);
	
	enum struct IncWnd : unsigned char { Left = 0x01, Right = 0x02, Up = 0x04, Down = 0x08, All = Left|Right|Up|Down, Horizontal=Left|Right, Vertical=Up|Down };
	static void onIncrementWindow(MSG* pMsg, int diff, IncWnd incDir = IncWnd::All, POINT* pCursorPos=nullptr, LRESULT hitTest = HTNOWHERE);
	static void onNcLButtonDown(MSG* pMsg);
	static void onLButtonUp(MSG* pMsg);
};

