#include "pch.h"
#include "WinPosWnd.h"
#include <string>
#include <ranges>
#include "Utils.h"
#include <algorithm>

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
		MONITORINFOEX mi = { sizeof(MONITORINFOEX) };
		GetMonitorInfo(hMon, &mi);
		mInfos.push_back(mi);
		return TRUE;
	}
}

/**
 * @brief 
 * @param monitorRects 
 * @param pt 
 * @return 
*/
RECT WinPosWnd::getTotalPreviewRect(const RectVector& monitorRects, POINT pt)
{
	RECT tpr = { 0 };
	for (const RECT& r : monitorRects)
	{
		RECT pr = tpr;
		UnionRect(&tpr, &pr, &r);
	}
	tpr = ScaleRect(tpr, F);
	OffsetRect(&tpr, pt.x - ((tpr.right - tpr.left) / 2), pt.y);
	return tpr;
}

/**
 * @brief 
 * @param monitorRects 
 * @param totalPreviewRect 
 * @param pt 
 * @return 
*/
WinPosWnd::RectVector WinPosWnd::getPreviewRects(const RectVector& monitorRects, RECT& totalRect, POINT& pt)
{
	using std::views::transform;
	using std::ranges::to;
	LONG xoffset = 0;
	return monitorRects | transform([&xoffset, &totalRect, pt](RECT r)
		{
			OffsetRect(&r, -r.left, 0);
			r = ScaleRect(r, F);
			OffsetRect(&r, pt.x - ((totalRect.right - totalRect.left) / 2) + xoffset, pt.y);
			xoffset += (r.right - r.left) - GetSystemMetrics(SM_CXBORDER);
			return  r;
		}) | to<RectVector>();
}

/**
 * @brief if the window looks beyond the edge of the monitor,
 *			 it is moved so that it is displayed as a whole on this monitor
 * @param previewRects 
 * @param pt 
 * @param totalRect 
*/
void WinPosWnd::correctEdgecases(RectVector& previewRects, POINT pt, const RECT& totalRect)
{
	HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		MONITORINFOEX mi = { sizeof(MONITORINFOEX) };
		GetMonitorInfo(hMonitor, &mi);
		const RECT& mr = mi.rcWork;

		using std::make_tuple;
		using std::get;
		enum { X, Y };

		auto offset = make_tuple(0, 0);

		if (totalRect.left < mr.left)
			offset = make_tuple(mr.left - totalRect.left, get<Y>(offset));

		if (totalRect.right > mr.right)
			offset = make_tuple(-(totalRect.right - mr.right), get<Y>(offset));

		if (totalRect.top < mr.top)
			offset = make_tuple(get<X>(offset), mr.top - totalRect.top);

		if (totalRect.bottom > mr.bottom)
			offset = make_tuple(get<X>(offset), -(totalRect.bottom - mr.bottom));

		if (offset != std::make_tuple(0, 0))
		{
			for (RECT& r : previewRects)
				OffsetRect(&r, get<X>(offset), get<Y>(offset));
		}
	}
}

/**
 * @brief 
 * @param pt 
 * @param hParentWnd 
*/
void WinPosWnd::create(POINT pt, HWND hParentWnd)
{
	if (wndClass == nullptr)
		wndClass = initWndClass();

	destroy();

	using MonitorInfos = std::vector<MONITORINFOEX>;
	MonitorInfos monitorInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&monitorInfos);
	
	using std::views::transform;
	using std::ranges::to;
	RectVector monitorRects = monitorInfos | transform([](const MONITORINFOEX& mi) { return mi.rcWork; }) | to<RectVector>();	
	
	RECT totalRect = getTotalPreviewRect(monitorRects, pt);
	RectVector previewRects = getPreviewRects(monitorRects, totalRect, pt);

	correctEdgecases(previewRects, pt, totalRect);


	//for (const MONITORINFOEX& mi : monitorInfos)
	for(const RECT& r : previewRects)
	{
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
}

/**
 * @brief 
*/
void WinPosWnd::destroy()
{
	for (HWND hWnd : _hWnds)
		DestroyWindow(hWnd);
	_hWnds.clear();
}

/**
 * @brief 
 * @param hWnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT WinPosWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_TIMER:
			if (wParam == CLOSE_TIMER)
			{
				destroy();
			}
			break;
		case WM_DESTROY:
		{
			WinPosWnd::hWndToWinPosMap.erase(hWnd);
			break;
		}
		case WM_LBUTTONUP:
			destroy();
			break;
		case WM_MOUSEMOVE:
			SetTimer(_hWnds.front(), CLOSE_TIMER, CLOSE_TIMEOUT, (TIMERPROC)nullptr);
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

