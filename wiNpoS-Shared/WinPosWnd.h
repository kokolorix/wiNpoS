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
	void create(POINT pt, HWND hParentWnd);

	void destroy();

private:

	static const WNDCLASS* wndClass;
	static std::map<HWND, WinPosWnd*> hWndToWinPosMap;

	std::vector<HWND>  _hWnds;

private:
	using RectVector = std::vector<RECT>;

	static WNDCLASS* initWndClass();
	static LRESULT CALLBACK WndPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	RECT getTotalPreviewRect(const RectVector& monitorRects, POINT pt);
	RectVector getPreviewRects(const RectVector& monitorRects, RECT& totalRect, POINT& pt);
	void correctEdgecases(RectVector& previewRects, POINT pt, const RECT& totalRect);

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

