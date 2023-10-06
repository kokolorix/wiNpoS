#include "pch.h"
#include "WinPosWnd.h"
#include <string>
#include <ranges>
#include "Utils.h"
#include <algorithm>
#include "WinPosWndConfig.h"
#include <corecrt_math.h>
#include <windowsx.h>
#include <assert.h>
#pragma comment(lib, "Msimg32.lib")

extern HINSTANCE hInstance;

namespace
{
	/**
	 * @brief ID from close timer
	*/
	const UINT CLOSE_TIMER = 0x30;
}
/**
 * @brief pointer to WNDCLASS structure from WinPosWnd
*/
const WNDCLASS* WinPosWnd::wndClass = nullptr;

/**
 * @brief map, contains all window handles for every instance of WinPosWnd
*/
std::map<HWND, WinPosWnd*> WinPosWnd::hWndToWinPosMap;

/**
 * @brief initialize the WNDCLASS structure
 * @return pointer to global WNDCLASS structure
*/
WNDCLASS* WinPosWnd::initWndClass()
{
	static const std::wstring className = L"wiNpoSWnd";
	static WNDCLASS wcl = { 0 };
	wcl.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hInstance = hInstance;
	wcl.lpszClassName = className.c_str();
	wcl.lpfnWndProc = WinPosWnd::WndProcStatic;

	WORD res = RegisterClass(&wcl);
	return &wcl;
}

/**
 * @brief static WNDPROC. Forwards to the WinPosWnd instances.
*/
LRESULT CALLBACK WinPosWnd::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WinPosWnd* winPosWnd = getWinPosWnd(hWnd))
		return winPosWnd->WndProc(hWnd, message, wParam, lParam);

	return DefWindowProc(hWnd, message, wParam, lParam);
}
/**
 * @brief the main WNDPROC for the WinPosWnd instances
*/
LRESULT WinPosWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using std::ranges::find_if;

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
		case WM_KEYDOWN:
		{
			WRITE_DEBUG_LOG_DETAIL("WM_KEYDOWN", format("Hwnd: {:#010x}, wParam: {}", (uint64_t)hWnd, wParam));
			if (wParam == VK_ESCAPE)
			{
				destroy();
				return  DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			auto it = find_if(_previews, [hWnd](auto& mp) { return mp->hWnd == hWnd; });
			if (it != _previews.end())
				(*it)->onLButtonDown(pt);

			destroy();
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (!_hWnds.empty())
				SetTimer(_hWnds.front(), CLOSE_TIMER, _closeTimeout, (TIMERPROC)nullptr);

			POINT pt = { 0 };
			GetCursorPos(&pt);

			for (auto& mp : _previews)
				mp->onMouseMove(pt);

			//WRITE_DEBUG_LOG_DETAIL("WM_MOUSEMOVE", format("Hwnd: {:#010x}, x:{}, y: {}", (uint64_t)hWnd, pt.x, pt.y));
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDc = BeginPaint(hWnd, &ps);

			auto it = find_if(_previews, [hWnd](auto mp) { return mp->hWnd == hWnd; });
			if (it != _previews.end())
				(*it)->paint(ps, hDc);

			EndPaint(hWnd, &ps);
			break;
		}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * @brief 
 * @param monitorRects 
 * @param pt 
 * @return 
*/
RECT WinPosWnd::getTotalPreviewRect(POINT pt, const RectVector& monitorRects)
{
	RECT tpr = { 0 };
	for (const RECT& r : monitorRects)
	{
		RECT pr = tpr;
		UnionRect(&tpr, &pr, &r);
	}
	tpr = ScaleRect(tpr, _scale);
	OffsetRect(&tpr, pt.x - ((tpr.right - tpr.left) / 2), pt.y);
	return tpr;
}

/**
 * @return the first window handle in the list. NULL if if none is available
*/
HWND WinPosWnd::getWndHandle()
{
	if (_hWnds.empty())
		return NULL;
	return _hWnds.front();
}

/**
 * @brief if the window looks beyond the edge of the monitor,
 *			 it is moved so that it is displayed as a whole on this monitor
 * @param pt 
 * @param totalRect 
*/
void WinPosWnd::correctEdgecases(POINT pt, const RECT& totalRect)
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
			for (auto& mp : _previews)
				OffsetRect(&mp->prvRect, get<X>(offset), get<Y>(offset));
		}
	}
}

/**
 * @brief get the instance of WinPosWnd to a window handle
 * @param hWnd 
*/
WinPosWnd* WinPosWnd::getWinPosWnd(HWND hWnd)
{	
	auto it = WinPosWnd::hWndToWinPosMap.find(hWnd);
	if (it != WinPosWnd::hWndToWinPosMap.end())
		return it->second;
	return nullptr;
}

/**
 * @brief create the required windows for monitor preview and all configured positions
 * @param pt point where the window should displayed 
 * @param hParentWnd the main window handle
*/
void WinPosWnd::create(POINT pt, HWND hParentWnd)
{
	using std::views::transform;
	using std::ranges::to;
	using MonitorConfig = WinPosWndConfig::MonitorConfig;

	if (wndClass == nullptr)
		wndClass = initWndClass();

	destroy();

	WinPosWndConfig config;
	config.readConfig();

	_closeTimeout = config.getCloseTimeout();
	_scale = config.getScale();

	auto monitorConfigs = config.getMonitors();

	RectVector monitorRects = monitorConfigs | transform([](const MonitorConfig& mc) { return mc.monitorRect; }) | to<RectVector>();	
	RECT totalRect = getTotalPreviewRect(pt, monitorRects);

	_previews = monitorConfigs | transform([pt, totalRect, this](const MonitorConfig& mc)
		{
			MonitorPreview mp = { *this, 0, mc.monitorRect, mc.previewRect, mc.name, mc.device };
			mp.offsetToPt(pt, totalRect);
			auto res = std::make_shared<MonitorPreview>(mp);
			res->createWinPosPreviews(mc.previews, res);
			return res;
		}) | to<MonitorPreviews>();

	correctEdgecases(pt, totalRect);

	_totalRect = totalRect;

	for (auto& mp : _previews)
	{
		HWND hWnd = mp->create(hParentWnd);
		_hWnds.push_back(hWnd);
		WinPosWnd::hWndToWinPosMap[hWnd] = this;
	}

	if(!_hWnds.empty())
	{
		//SetCapture(_hWnds.front());
		SetTimer(_hWnds.front(), CLOSE_TIMER, _closeTimeout, (TIMERPROC)nullptr);
	}
}

/**
 * @brief destroys every windows
*/
void WinPosWnd::destroy()
{
	for (HWND hWnd : _hWnds)
		DestroyWindow(hWnd);
	_hWnds.clear();
}

/**
 * @brief move the rectangle to the cursor position
 * @param pt cursor position, screen coordinates
 * @param totalRect of all previewed monitors
*/
void WinPosWnd::MonitorPreview::offsetToPt(POINT pt, RECT totalRect)
{
	OffsetRect(&prvRect, pt.x - ((totalRect.right - totalRect.left) / 2), pt.y + GetSystemMetrics(SM_CYCAPTION));
}

/**
 * @brief creates all configured positions for this monitor preview
 * @param pcs PosPreviewConfigs
 * @param mp pointer to monitor preview instance 
*/
void WinPosWnd::MonitorPreview::createWinPosPreviews(const PosPreviewConfigs& pcs, MonitorPreviewPtr mp)
{
	using std::views::transform;
	using std::ranges::to;
	using PosPreviewConfig = WinPosWndConfig::PosPreviewConfig;


	previews = pcs | transform([this, mp](const PosPreviewConfig& pc)
		{
			static auto getRect =
				[this, pc](RECT r, RECT mr)->RECT
				{
					double x1percent = (mr.right - mr.left) / 100.0;
					double y1Percent = (mr.bottom - mr.top) / 100.0;
					RECT res =
						pc.units == Pixels ?
						RECT{ r.left, r.top, r.left + r.right, r.top + r.bottom } :
						RECT{
							(LONG)std::round(x1percent * r.left),
							(LONG)std::round(y1Percent * r.top),
							(LONG)std::round((x1percent * r.left) + (x1percent * r.right)),
							(LONG)std::round((y1Percent * r.top) + (y1Percent * r.bottom)),
						};
					return res;
				};

			WinPosPreview wp = {
				getRect(pc.wndRect, monitorRect),
				getRect(pc.prvRect, prvRect),
				pc.name,
				mp,
			};
			auto res = std::make_shared<WinPosPreview>(wp);
			return res;
		}) | to<WinPosPreviews>();
}

/**
 * @brief calculates the close cross in top right
*/
RECT WinPosWnd::MonitorPreview::getCrossRect()
{
	RECT rc = prvRect;
	OffsetRect(&rc, -rc.left, -rc.top);
	int size = GetSystemMetrics(SM_CXMENUSIZE);
	RECT rcCross = { rc.right - size, rc.top, rc.right, size };
	return rcCross;
}

/**
 * @brief create the window for a monitor preview
 * @param hParentWnd the main window
 * @return 
*/
HWND WinPosWnd::MonitorPreview::create(HWND hParentWnd)
{
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST, // | WS_EX_TOOLWINDOW,// Optional window styles.
		wndClass->lpszClassName, // Window class
		L"Pick a rectangle",     // Window text
		WS_POPUP | WS_VISIBLE | WS_BORDER, //WS_OVERLAPPEDWINDOW,   // Window style
		// Size and position
		prvRect.left,
		prvRect.top,
		prvRect.right - prvRect.left,
		prvRect.bottom - prvRect.top,

		hParentWnd,	// Parent window
		NULL,			// Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);
	return hWnd;
}

/**
 * @brief find the top WinPosPreview
 * @param pt current mouse cursor 
*/
WinPosWnd::WinPosPreviewPtr WinPosWnd::MonitorPreview::findPreview(POINT pt)
{
	//MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&pt, 1);
	WinPosPreviewPtr wp;
	using std::ranges::find_if;
	using std::views::reverse;
	auto rpreviews = reverse(previews);
	auto it = find_if(rpreviews,
		[this, pt](auto wp)
		{
			RECT rc = wp->prvRect;
			MapWindowPoints(hWnd, HWND_DESKTOP, (LPPOINT)&rc, 2);
			return PtInRect(&rc, pt);
		});

	if (it != rpreviews.end())
		wp = *it;
	return wp;
}

/**
 * @brief paint procedure for monitor preview
 * @param ps 
 * @param hDc 
*/
void WinPosWnd::MonitorPreview::paint(PAINTSTRUCT& ps, HDC hDc)
{
	HFONT hFont = CreateFontA(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	HFONT hCurrentFont = (HFONT)SelectObject(hDc, hFont);

	RECT rc = prvRect;
	OffsetRect(&rc, -rc.left, -rc.top);
	RECT rcText = rc;
	int oldBkMode = SetBkMode(hDc, TRANSPARENT);	
	COLORREF oldColor = SetTextColor(hDc, GetSysColor(COLOR_GRAYTEXT)); 

	string text = format("{}\n{}", name, device);
	int textHeight = DrawTextA(hDc, text.c_str(), text.length(), &rcText, DT_CALCRECT|DT_CENTER|DT_VCENTER|DT_WORDBREAK);
	OffsetRect(&rc, 0, (rc.bottom - textHeight) / 2);
	DrawTextA(hDc, text.c_str(), text.length(), &rc, DT_CENTER|DT_VCENTER|DT_WORDBREAK);

	SetBkMode(hDc, oldBkMode);
	SetTextColor(hDc, oldColor);

	for (auto pv : previews)
		pv->paint(hWnd, ps, hDc);

	if (winPosWnd._previews.back().get() == this)
	{
		RECT rcCross = getCrossRect();
		POINT pt = { 0 };
		GetCursorPos(&pt);
		MapWindowPoints(HWND_DESKTOP, hWnd, &pt, 1);
		if (PtInRect(&rcCross, pt))
		{
			FillRect(hDc, &rcCross, GetSysColorBrush(COLOR_ACTIVECAPTION));
			activeWinPosPreview.reset();
		}
		else
			FillRect(hDc, &rcCross, GetSysColorBrush(COLOR_INACTIVECAPTION));

		int thickness = GetSystemMetrics(SM_CXBORDER) + 1;
		int edge = thickness * 2;
		HPEN hPen = CreatePen(PS_SOLID, thickness, GetSysColor(COLOR_HOTLIGHT));
		HPEN hOldPen = (HPEN)SelectObject(hDc, hPen);

		MoveToEx(hDc, rcCross.left + edge, rcCross.top + edge, NULL); // Starting point
		LineTo(hDc, rcCross.right - edge, rcCross.bottom - edge); // Ending point
		MoveToEx(hDc, rcCross.right - edge, rcCross.top + edge, NULL); // Starting point
		LineTo(hDc, rcCross.left + edge, rcCross.bottom - edge); // Ending point	

		// Clean up
		SelectObject(hDc, hOldPen);
		DeleteObject(hPen);

		FrameRect(hDc, &rcCross, GetSysColorBrush(COLOR_HOTLIGHT));
	}

	if (hCurrentFont)
		SelectObject(hDc, hCurrentFont);
	if (hFont)
		DeleteObject(hFont);

}

/**
 * @brief select the active element, with every mouse movement  
 * @param pt in screen coordinates
*/
void WinPosWnd::MonitorPreview::onMouseMove(POINT pt)
{
	{
		RECT rcCross = getCrossRect();
		POINT ptClient = pt;
		MapWindowPoints(HWND_DESKTOP, hWnd, &ptClient, 1);
		if (PtInRect(&rcCross, ptClient))
		{
			activeWinPosPreview.reset();
			RECT rcWnd = { 0 };
			GetClientRect(hWnd, &rcWnd);
			InvalidateRect(hWnd, &rcWnd, TRUE);
			return;
		}
	}

	WinPosPreviewPtr wp = findPreview(pt);
	if (activeWinPosPreview != wp)
	{
		activeWinPosPreview = wp;
		RECT rcWnd = { 0 };
		GetClientRect(hWnd, &rcWnd);
		InvalidateRect(hWnd, &rcWnd, TRUE);
	}

	if (activeWinPosPreview)
		activeWinPosPreview->onMouseMove(hWnd, pt);
}

/**
 * @brief the main function 
 * @param pt in screen coordinates 
*/
void WinPosWnd::MonitorPreview::onLButtonDown(POINT pt)
{
	//WinPosPreviewPtr wp = findPreview(pt);
	if (activeWinPosPreview)
		activeWinPosPreview->onLButtonDown(hWnd, pt);
}

/**
 * @brief paint procedure for WinPosPreview 
 * @param hWnd 
 * @param ps 
 * @param hDc 
*/
void WinPosWnd::WinPosPreview::paint(HWND hWnd, PAINTSTRUCT& ps, HDC hDc) const
{
	auto mp = monitorPreview.lock();
	if(mp->activeWinPosPreview.get() == this)
	{
		FillRect(hDc, &prvRect, GetSysColorBrush(COLOR_ACTIVECAPTION));
	}
	else
	{
		// Create a semi-transparent color using ALPHA
		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 128 + 64; // 0 (transparent) to 255 (opaque)

		// Draw the half-transparent rectangle
		HDC memDC = CreateCompatibleDC(hDc);
		HBITMAP memBitmap = CreateCompatibleBitmap(hDc, prvRect.right - prvRect.left, prvRect.bottom - prvRect.top);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

		FillRect(memDC, &prvRect, GetSysColorBrush(COLOR_INACTIVECAPTION));
		AlphaBlend(hDc, prvRect.left, prvRect.top, prvRect.right - prvRect.left, prvRect.bottom - prvRect.top, memDC, 0, 0, prvRect.right - prvRect.left, prvRect.bottom - prvRect.top, blendFunction);

		SelectObject(memDC, hOldBitmap);
		DeleteObject(memBitmap);
		DeleteDC(memDC);
		//FillRect(hDc, &prvRect, GetSysColorBrush(COLOR_INACTIVECAPTION));
	}
	FrameRect(hDc, &prvRect, GetSysColorBrush(COLOR_HOTLIGHT));

	RECT rc = prvRect;
	RECT rcText = rc;

	int oldBkMode = SetBkMode(hDc, TRANSPARENT);	
	COLORREF oldColor = SetTextColor(hDc, GetSysColor(COLOR_GRAYTEXT)); 
	
	int textHeight = DrawTextA(hDc, name.c_str(), name.length(), &rcText, DT_CALCRECT|DT_CENTER|DT_VCENTER|DT_WORDBREAK);
	OffsetRect(&rc, 0, (rc.bottom - rc.top - textHeight) / 2);
	DrawTextA(hDc, name.c_str(), name.length(), &rc, DT_CENTER|DT_VCENTER|DT_WORDBREAK);

	//if (hCurrentFont)
	//	SelectObject(hDc, hCurrentFont);
	//if (hFont)
	//	DeleteObject(hFont);

	SetBkMode(hDc, oldBkMode);
	SetTextColor(hDc, oldColor);

}

/**
 * @brief handles mouse movement  
 * @param hWnd 
 * @param pt 
*/
void WinPosWnd::WinPosPreview::onMouseMove(HWND hWnd, POINT pt)
{

}

/**
 * @brief main function of WinPosPreview
 * @param hWnd 
 * @param pt 
*/
void WinPosWnd::WinPosPreview::onLButtonDown(HWND hWnd, POINT pt)
{
	HWND hParent = GetParent(hWnd);

	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hParent, &wp);
	wp.flags = WPF_ASYNCWINDOWPLACEMENT;
	wp.rcNormalPosition = wndRect;
	MonitorPreviewPtr mp = monitorPreview.lock();
	RECT mr = mp->monitorRect;
	OffsetRect(&wp.rcNormalPosition, mr.left, mr.top);
	wp.showCmd = SW_NORMAL;
	BOOL wndReplaced = SetWindowPlacement(hParent, &wp);
	assert(wndReplaced);
}
