#pragma once
#include "WinPosWnd.h"
#include <chrono>

class HooksImpl
{
public:
	static LRESULT CALLBACK shellHookProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK callWndProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK getMsgProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);

	MSG* getLButtonUpMsg() { return &_lButtonUpMsg; }
	MSG* getNcLButtonUpMsg() { return &_ncLButtonUpMsg; }

private:
	WinPosWnd _winPosWnd;
	MSG _lButtonUpMsg = { 0 };
	MSG _ncLButtonUpMsg = { 0 };
	static std::chrono::system_clock::time_point lastClick;


	POINT _lastLButtonDown = { 0 };
	RECT _lastRect = { 0 }; // last rect
	bool hasLastRect()
	{
		return _lastRect.left + _lastRect.right + _lastRect.right + _lastRect.bottom != 0;
	}
	POINT _captionDblClick = { 0 }; // caption dbl clicked
	bool hasCaptionDblClicked()
	{
		return _captionDblClick.x + _captionDblClick.y != 0;
	}


private:
	void onNcLButtonDblClick(MSG* pMsg);
	void onWindowPosChanged(CWPSTRUCT* pMsg);
	
	enum struct IncWnd : unsigned char { Left = 0x01, Right = 0x02, Up = 0x04, Down = 0x08, All = Left|Right|Up|Down, Horizontal=Left|Right, Vertical=Up|Down };
	void onIncrementWindow(MSG* pMsg, int diff, IncWnd incDir = IncWnd::All, POINT* pCursorPos = nullptr, LRESULT hitTest = HTNOWHERE, bool async = true);
	void onNcLButtonDown(MSG* pMsg);
	void onNCLButtonUp(MSG* pMsg);

	void onCaptionClick(MSG* pMsg);

	void onLButtonUp(MSG* pMsg);
	bool onClosePosWnd(MSG* pMsg, POINT pt);
	HWND onShowPosWnd(MSG* pMsg, POINT pt);
};
