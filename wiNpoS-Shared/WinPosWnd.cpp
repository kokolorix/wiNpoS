#include "pch.h"
#include "WinPosWnd.h"
#include <string>
#include <ranges>
#include "Utils.h"

extern HINSTANCE hInstance;

namespace
{
	FLOAT F = 0.12f; // factor
	const UINT CLOSE_TIMER = 0x30;
	UINT CLOSE_TIMEOUT = 3500;
}

const WNDCLASS* WinPosWnd::wndClass = nullptr;

std::map<HWND, WinPosWnd*> WinPosWnd::hWndToWinPosMap;

WNDCLASS* WinPosWnd::initWndClass()
{
	static const std::wstring className = L"wiNpoSWnd";
	static WNDCLASS wcl = { 0 };
	wcl.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hInstance = hInstance;
	wcl.lpszClassName = className.c_str();
	wcl.lpfnWndProc = WinPosWnd::WndPosWndProc;

	WORD res = RegisterClass(&wcl);
	return &wcl;
}

LRESULT CALLBACK WinPosWnd::WndPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto it = WinPosWnd::hWndToWinPosMap.find(hWnd);
	if(it != WinPosWnd::hWndToWinPosMap.end())
		return it->second->WndProc(hWnd, message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}
namespace
{
	BOOL CALLBACK Monitorenumproc(HMONITOR hMon, HDC hDC, LPRECT pRECT, LPARAM lParam)
	{
		std::vector<MONITORINFOEX>& mInfos = *(std::vector<MONITORINFOEX>*)lParam;
		MONITORINFOEX mi = { 0 };
		mi.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMon, &mi);
		mInfos.push_back(mi);
		return TRUE;
	}
}
RECT WinPosWnd::calculateWndRect(POINT pt)
{
	std::vector<MONITORINFOEX> monInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&monInfos);

	using std::views::transform;
	//using std::views::filter;
	using std::ranges::min;
	using std::ranges::max;

	// determine offset from screens with different resolutions
	LONG ox = min(monInfos | transform([](const MONITORINFOEX& mi) { return mi.rcWork.left; }));
	LONG oy = min(monInfos | transform([](const MONITORINFOEX& mi) {return mi.rcWork.top; }));


	return { pt.x - 150, pt.y, pt.x + 150, pt.y + 300 };
}

void WinPosWnd::show(POINT pt, HWND hParentWnd)
{
	if (wndClass == nullptr)
		wndClass = initWndClass();

	for (HWND hWnd : _hWnds)
		DestroyWindow(hWnd);
	_hWnds.clear();

	std::vector<MONITORINFOEX> monInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&monInfos);

	LONG xoffset = 0;
	for (const MONITORINFOEX& mi : monInfos)
	{
		RECT r = mi.rcWork;
		OffsetRect(&r, -r.left, 0);
		r = ScaleRect(r, F);
		OffsetRect(&r, pt.x + xoffset, pt.y);
		xoffset += (r.right - r.left) - GetSystemMetrics(SM_CXBORDER);

		HWND hWnd = CreateWindowEx(
			WS_EX_TOPMOST, // | WS_EX_TOOLWINDOW,// Optional window styles.
			wndClass->lpszClassName,                     // Window class
			L"Pick a rectangle",    // Window text
			WS_POPUP | WS_VISIBLE | WS_BORDER, //
			//WS_OVERLAPPEDWINDOW, // Window style

			// Size and position
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			r.left, r.top, r.right - r.left, r.bottom - r.top,

			hParentWnd,		// Parent window
			NULL,			// Menu
			hInstance, // Instance handle
			NULL        // Additional application data
		);
		_hWnds.push_back(hWnd);
		WinPosWnd::hWndToWinPosMap[hWnd] = this;
	}
	SetTimer(_hWnds.front(), CLOSE_TIMER, CLOSE_TIMEOUT, (TIMERPROC)nullptr);
	//RECT rct = calculateWndRect(pt);

}

LRESULT WinPosWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_TIMER:
			if (wParam == CLOSE_TIMER)
			{
				for (HWND hWnd : _hWnds)
					DestroyWindow(hWnd);
				_hWnds.clear();
			}
			break;
		case WM_DESTROY:
		{
			WinPosWnd::hWndToWinPosMap.erase(hWnd);
			break;
		}
		case WM_LBUTTONUP:
			for (HWND hWnd : _hWnds)
				DestroyWindow(hWnd);
			_hWnds.clear();
			break;
		case WM_MOUSEMOVE:
			SetTimer(_hWnds.front(), CLOSE_TIMER, CLOSE_TIMEOUT, (TIMERPROC)nullptr);
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

