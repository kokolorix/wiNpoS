#pragma once
#include <wtypes.h>
#include <map>
#include <vector>
/**
 * @brief 
*/
class WinPosWnd
{
public:
	void show(POINT pt, HWND hParentWnd);

private:

	static const WNDCLASS* wndClass;
	static std::map<HWND, WinPosWnd*> hWndToWinPosMap;

	std::vector<HWND>  _hWnds;

private:
	static WNDCLASS* initWndClass();
	static LRESULT CALLBACK WndPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	RECT calculateWndRect(POINT pt);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

