#include "pch.h"
#include "DebugNew.h"
#include "HooksImpl.h"
#include "HooksMgr.h"
#include "Utils.h"
#include "TaskToolbar.h"
#include <Shlwapi.h>
#include <windowsx.h>
#include <resource.h>
#include <assert.h>
#include <future>

extern HINSTANCE hInstance;
thread_local HRESULT coInit /*= S_FALSE*/;

thread_local TaskToolbar _thumbnailToolbar;
thread_local HooksImpl _hooks;

namespace
{
	/**
	 * @brief ID from close timer
	*/
	const UINT TI_LBUTTONUP = 0x40;
	const UINT TI_NCLBUTTONUP = 0x50;
}

/**
 * @brief  the exported hook for the shell procedure
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return HooksImpl::shellHookProc(nCode, wParam, lParam);
}
/**
 * @brief the exported hook for the main window procedure
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK CallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::callWndProc(nCode, wParam, lParam);
}
/**
 * @brief the exported hook for the windows-message procedure
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK GetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::getMsgProc(nCode, wParam, lParam);

}
/**
 * @brief 
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK HooksImpl::shellHookProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	if (nCode == HSHELL_WINDOWCREATED)
	{
		HWND hNewWindow = (HWND)wParam;
		PostMessage(hNewWindow, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
/**
 * @brief here we follow the main message loop of the application
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK HooksImpl::callWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (nCode)
	{
		case HC_ACTION:
		{
			CWPSTRUCT* pMsg = (CWPSTRUCT*)lParam;

			switch (pMsg->message)
			{
				//case WM_MOUSEMOVE:
				//	WRITE_DEBUG_LOG(format("WM_MOUSEMOVE: {:#010x} ", pMsg->message));
				//	break;
				//case WM_SHOWWINDOW:
				//	WRITE_DEBUG_LOG(format("WM_SHOWWINDOW: {:#010x} ", pMsg->message));
				//	break;
				case WM_WINDOWPOSCHANGED:
					//WRITE_DEBUG_LOG(format("WM_WINDOWPOSCHANGED: {:#010x} ", pMsg->message));
					_hooks.onWindowPosChanged(pMsg);
					break;
				default:
					break;
			}
			//WRITE_DEBUG_LOG(format("Msg: {} in {}", pMsg->message, Utils::ExeName));
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
/**
 * @brief here the Windows messages are picked up, which do not appear in the main message loop 
 * @param nCode 
 * @param wParam 
 * @param lParam 
 * @return 
*/
LRESULT CALLBACK HooksImpl::getMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (nCode)
	{
		case HC_ACTION:
		{
			if (wParam == PM_REMOVE || wParam == PM_NOREMOVE || wParam == PM_NOYIELD)
			{
				MSG* pMsg = (MSG*)lParam;

				switch (pMsg->message)
				{
					//case WM_MOUSEMOVE:
					//	WRITE_DEBUG_LOG(format("WM_MOUSEMOVE: {:#010x} ", pMsg->message));
					//	break;
					case WM_SHOWWINDOW:
						WRITE_DEBUG_LOG(dformat("WM_SHOWWINDOW: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_GETMINMAXINFO:
						WRITE_DEBUG_LOG(dformat("WM_GETMINMAXINFO: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_WINDOWPOSCHANGING:
						WRITE_DEBUG_LOG(dformat("WM_WINDOWPOSCHANGING: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_LBUTTONDBLCLK:
						WRITE_DEBUG_LOG(dformat("WM_LBUTTONDBLCLK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_NCRBUTTONDOWN:
						WRITE_DEBUG_LOG(dformat("WM_NCRBUTTONDOWN: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_NCRBUTTONUP:
						WRITE_DEBUG_LOG(dformat("WM_NCRBUTTONUP: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						break;
					case WM_NCLBUTTONDOWN:
						WRITE_DEBUG_LOG(dformat("WM_NCLBUTTONDOWN: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						_hooks.onClosePosWnd(pMsg, { GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) });
						_hooks.onNcLButtonDown(pMsg);
						break;
					case WM_NCLBUTTONUP:
						WRITE_DEBUG_LOG(dformat("WM_NCLBUTTONUP: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						_hooks.onNCLButtonUp(pMsg);
						break;
					case WM_NCLBUTTONDBLCLK:
						WRITE_DEBUG_LOG(dformat("WM_NCLBUTTONDBLCLK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						_hooks.onNcLButtonDblClick(pMsg);
						break;
					case WM_LBUTTONDOWN:
						WRITE_DEBUG_LOG(dformat("WM_LBUTTONDOWN: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						_hooks.onClosePosWnd(pMsg, { GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) });
						break;
					case WM_LBUTTONUP:
						WRITE_DEBUG_LOG(dformat("WM_LBUTTONUP: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						//SetTimer(pMsg->hwnd, TI_NCLBUTTONUP, USER_TIMER_MINIMUM, (TIMERPROC)nullptr);
						//PostMessage(pMsg->hwnd, DO_THE_WORK, (WPARAM)GetCurrentThreadId(), 0);
						_hooks.onLButtonUp(pMsg);
						break;
					case WM_KEYDOWN:
						//WRITE_DEBUG_LOG(dformat("WM_KEYDOWN: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						if (pMsg->wParam == VK_ESCAPE)
						{
							_hooks.onClosePosWnd(pMsg, { 0 });
						}
						break;
					case WM_TIMER:
						if (pMsg->wParam == TI_LBUTTONUP)
						{
							WRITE_DEBUG_LOG(dformat("WM_TIMER: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
							KillTimer(pMsg->hwnd, TI_LBUTTONUP);
							MSG msg = *_hooks.getLButtonUpMsg();

							// extract the exact click point from lParam
							POINT pt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
							// Adjust the client rectangle to screen coordinates
							MapWindowPoints(msg.hwnd, HWND_DESKTOP, (LPPOINT)&pt, 1);
							// check if the click is on a control of the title bar
							LRESULT hitTest = SendMessage(msg.hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));

							msg.lParam = MAKELPARAM(pt.x, pt.y);
							msg.wParam = hitTest;

							_hooks.onCaptionClick(&msg);
						}
						else if (pMsg->wParam == TI_NCLBUTTONUP)
						{
							WRITE_DEBUG_LOG(dformat("WM_TIMER: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
							KillTimer(pMsg->hwnd, TI_NCLBUTTONUP);
							_hooks.onCaptionClick(_hooks.getNcLButtonUpMsg());
						}
						break;

					case WM_COMMAND:
					{
						uint16_t type = HIWORD(pMsg->wParam);
						uint16_t wmId = LOWORD(pMsg->wParam);

						WRITE_DEBUG_LOG(format("WM_COMMAND: {:#010x} ID: {}", pMsg->message, wmId));

						switch (wmId)
						{
							case ID_SYSMENU_INCREMENTWINDOWSIZE:
								_hooks.onIncrementWindow(pMsg, 10);
								break;
							case ID_SYSMENU_DECREMENTWINDOWSIZE:
								_hooks.onIncrementWindow(pMsg, -10);
								break;
							case ID_SYSMENU_SHOWPOSWINDOW:
							{
								POINT pt = { 0 };
								GetCursorPos(&pt);
								HWND hWnd = _hooks.onShowPosWnd(pMsg, pt);
								break;
							}
							default:
								break;
						}
					}
					break;
					default:
					{
						if (pMsg->message == MT_HOOK_MSG_CREATE_TASK_TOOLBAR)
						{
							WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_CREATE_TASK_TOOLBAR: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));

							HWND hMainWnd = pMsg->hwnd; // GetAncestor(pMsg->hwnd, GA_ROOT);

							HRESULT hr = _thumbnailToolbar.initialize(hInstance, hMainWnd);
							if (hr == 0x800401f0)//0x800401f0 : CoInitialize has not been called.
							{
								coInit = CoInitialize(NULL);
								hr = _thumbnailToolbar.initialize(hInstance, hMainWnd);
							}
						}

						else if (pMsg->message == MT_HOOK_MSG_DESTROY_TASK_TOOLBAR)
						{
							WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_DESTROY_TASK_TOOLBAR: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
							HWND hMainWnd = pMsg->hwnd; // GetAncestor(pMsg->hwnd, GA_ROOT);
							_thumbnailToolbar.uninitialize(hInstance, hMainWnd);
						}
						break;
					}
				}

				//WRITE_DEBUG_LOG(format("Msg: {} in {}", pMsg->message, Utils::ExeName));
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
/**
 * @brief this procedure handles the double click in non-client area
 * @param pMsg 
*/
void HooksImpl::onNcLButtonDblClick(MSG* pMsg)
{
	_lastLButtonDown = { 0 };

	int x = GET_X_LPARAM(pMsg->lParam);
	int y = GET_Y_LPARAM(pMsg->lParam);

	POINT pt = { 0 };
	GetCursorPos(&pt);
	LRESULT hitTest = SendMessage(pMsg->hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));

	// Get the window's client rectangle
	RECT cr;
	GetClientRect(pMsg->hwnd, &cr);

	RECT wr;
	GetWindowRect(pMsg->hwnd, &wr);

	// Adjust the client rectangle to screen coordinates
	MapWindowPoints(pMsg->hwnd, HWND_DESKTOP, (LPPOINT)&cr, 2);

	// Check if the double-click occurred in the border area
	//if (pt.x < cr.left || pt.x >= cr.right)
	if (is_one_of(hitTest, HTLEFT, HTRIGHT))
	{
		if (hasLastRect())
		{
			auto resFuture = std::async(std::launch::async, [pMsg, this]()
				{
					WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#010x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, _lastRect.left, _lastRect.top, _lastRect.right - _lastRect.left, _lastRect.bottom - _lastRect.top, SWP_NOZORDER));
					SetWindowPos(pMsg->hwnd, NULL, _lastRect.left, _lastRect.top, _lastRect.right - _lastRect.left, _lastRect.bottom - _lastRect.top, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
					_lastRect = { 0 };
				});
			//SetWindowPos(pMsg->hwnd, NULL, _lastRect.left, _lastRect.top, _lastRect.right - _lastRect.left, _lastRect.bottom - _lastRect.top, SWP_NOZORDER);
			//_lastRect = { 0 };
		}
		else
		{
			// Get the handle to the primary monitor
			HMONITOR hMonitor = MonitorFromWindow(pMsg->hwnd, MONITOR_DEFAULTTOPRIMARY);

			// Get the monitor information
			MONITORINFOEX mi = { sizeof(MONITORINFOEX) };
			mi.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(hMonitor, &mi);

			// Calculate the width of the monitor
			int ml = mi.rcMonitor.left;
			int mw = mi.rcMonitor.right - mi.rcMonitor.left;
			_lastRect = wr;
			//WINUSERAPI BOOL WINAPI SetWindowPos(_In_ HWND hWnd, _In_opt_ HWND hWndInsertAfter, _In_ int X, _In_ int Y, _In_ int cx, _In_ int cy, _In_ UINT uFlags)
			auto resFuture = std::async(std::launch::async, [ml, wr, mw, pMsg, this]()
				{
					WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#010x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, ml, wr.top, mw, wr.bottom - wr.top, SWP_NOZORDER));
					SetWindowPos(pMsg->hwnd, NULL, ml, wr.top, mw, wr.bottom - wr.top, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
				});
		}
	}
	// Check if the double-click occurred in the caption area
	int lowerCaption = wr.top + GetSystemMetrics(SM_CYCAPTION);
	if (pt.y < lowerCaption)
	{
		bool isRelevant = hitTest != HTMINBUTTON && hitTest != HTMAXBUTTON && hitTest != HTCLOSE && hitTest != HTSYSMENU;

		if (hasLastRect() && isRelevant)
		{
			_captionDblClick = pt;
		}
	}
}
/**
 * @brief here we react to position changes of the main window
 * @param pMsg 
*/
void HooksImpl::onWindowPosChanged(CWPSTRUCT* pMsg)
{
	if (hasCaptionDblClicked())
	{
		_captionDblClick = { 0 };

		if (hasLastRect())
		{
			if (IsZoomed(pMsg->hwnd))
			{
				WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#018x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, _lastRect.left, _lastRect.top, _lastRect.right - _lastRect.left, _lastRect.bottom - _lastRect.top, SWP_NOZORDER));
				ShowWindow(pMsg->hwnd, SW_NORMAL);
				SetWindowPos(pMsg->hwnd, NULL, _lastRect.left, _lastRect.top, _lastRect.right - _lastRect.left, _lastRect.bottom - _lastRect.top, SWP_NOZORDER);
				_lastRect = { 0 };
			}
		}
	}
}
/**
 * @brief processes the WM_NCLBUTTONDOWN event
 * @param pMsg 
*/
void HooksImpl::onNcLButtonDown(MSG* pMsg)
{
	int x = GET_X_LPARAM(pMsg->lParam);
	int y = GET_Y_LPARAM(pMsg->lParam);
	POINT pt = { x, y };
	LRESULT hitTest = SendMessage(pMsg->hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));

	if (!is_one_of(hitTest, HTMINBUTTON, HTMAXBUTTON, HTCLOSE, HTSYSMENU, HTLEFT, HTRIGHT, HTTOP, HTBOTTOM))
	{
		_lastLButtonDown = pt;
	}
}
/**
 * @brief processes the WM_NCLBUTTONUP event
 * @param pMsg 
*/
void HooksImpl::onNCLButtonUp(MSG* pMsg)
{
	_ncLButtonUpMsg = *pMsg;
	UINT dblClickTime = GetDoubleClickTime();
	SetTimer(pMsg->hwnd, TI_NCLBUTTONUP, dblClickTime, (TIMERPROC)nullptr);
}
/**
 * @brief processes the WM_LBUTTONUP event
 * @param pMsg 
*/
void HooksImpl::onLButtonUp(MSG* pMsg)
{
	// extract the exact click point from lParam
	POINT pt = { GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) };
	// Adjust the client rectangle to screen coordinates
	MapWindowPoints(pMsg->hwnd, HWND_DESKTOP, (LPPOINT)&pt, 1);
	// check if the click is on a control of the title bar
	LRESULT hitTest = SendMessage(pMsg->hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));

	// check the Key States
	bool lCtrl = HIBYTE(GetAsyncKeyState(VK_LCONTROL));
	bool lShift = HIBYTE(GetAsyncKeyState(VK_LSHIFT));
	bool lAlt= HIBYTE(GetAsyncKeyState(VK_LMENU));
	//bool rctrl = HIBYTE(GetAsyncKeyState(VK_RCONTROL));
	//bool rshift =HIBYTE(GetAsyncKeyState(VK_RSHIFT));
	//bool ralt =HIBYTE(GetAsyncKeyState(VK_RMENU));
	//bool left = HIBYTE(GetAsyncKeyState(VK_LEFT));
	//bool down = HIBYTE(GetAsyncKeyState(VK_DOWN));
	//bool right = HIBYTE(GetAsyncKeyState(VK_RIGHT));
	//bool up = HIBYTE(GetAsyncKeyState(VK_UP));

	if (!is_one_of(hitTest, HTMINBUTTON, HTMAXBUTTON, HTCLOSE, HTSYSMENU, HTLEFT, HTRIGHT, HTTOP, HTBOTTOM) && !hasCaptionDblClicked())
	{
		_lButtonUpMsg = *pMsg;
		UINT dblClickTime = GetDoubleClickTime();
		SetTimer(pMsg->hwnd, TI_LBUTTONUP, dblClickTime, (TIMERPROC)nullptr);

		//static const int32_t TOL = 3; // tolerance
		//RECT rc = { pt.x - TOL, pt.y - TOL, pt.x + TOL, pt.y + TOL };
		//if (PtInRect(&rc, lb.lastLButtonDown))
		//{
		//	lb.lastLButtonDown = { 0 };
		//	if (lCtrl)
		//		onIncrementWindow(pMsg, 10, IncWnd::All, &pt, HTCAPTION);
		//	else if (lShift)
		//		onIncrementWindow(pMsg, -10, IncWnd::All, &pt, HTCAPTION);
		//	else
		//		onShowPosWnd(pMsg, pt);
		//}
	}
	else if (is_one_of(hitTest, HTLEFT) && lCtrl && lAlt)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Left, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTLEFT) && lShift && lAlt)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Left, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTRIGHT) && lCtrl && lAlt)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Right, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTRIGHT) && lShift && lAlt)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Right, &pt, hitTest);
	}

	else if (is_one_of(hitTest, HTTOP) && lCtrl && lAlt)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Up, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTTOP) && lShift && lAlt)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Up, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTBOTTOM) && lCtrl && lAlt)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Down, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTBOTTOM) && lShift && lAlt)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Down, &pt, hitTest);
	}

	else if (is_one_of(hitTest, HTLEFT, HTRIGHT) && lCtrl)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Horizontal, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTLEFT, HTRIGHT) && lShift)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Horizontal, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTTOP, HTBOTTOM) && lCtrl)
	{
		onIncrementWindow(pMsg, 10, IncWnd::Vertical, &pt, hitTest);
	}
	else if (is_one_of(hitTest, HTTOP, HTBOTTOM) && lShift)
	{
		onIncrementWindow(pMsg, -10, IncWnd::Vertical, &pt, hitTest);
	}
}

/**
 * @brief do the work, if necessary
 * @param pMsg 
*/
void HooksImpl::onCaptionClick(MSG* pMsg)
{
	// extract the exact click point from lParam
	POINT pt = { GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) };
	LRESULT hitTest = pMsg->wParam; // and the hitTest from wParam

	// check the Key States
	bool lCtrl = HIBYTE(GetAsyncKeyState(VK_LCONTROL));
	bool lShift = HIBYTE(GetAsyncKeyState(VK_LSHIFT));
	bool lAlt = HIBYTE(GetAsyncKeyState(VK_LMENU));

	if (!is_one_of(hitTest, HTMINBUTTON, HTMAXBUTTON, HTCLOSE, HTSYSMENU, HTLEFT, HTRIGHT, HTTOP, HTBOTTOM) && !hasCaptionDblClicked())
	{
		int xTol = GetSystemMetrics(SM_CXDOUBLECLK) / 2, yTol = GetSystemMetrics(SM_CYDOUBLECLK) / 2;
		RECT rc = { pt.x - xTol, pt.y - yTol, pt.x + xTol, pt.y + yTol };
		if (PtInRect(&rc, _lastLButtonDown))
		{
			_lastLButtonDown = { 0 };
			if (lCtrl)
				onIncrementWindow(pMsg, 10, IncWnd::All, &pt, HTCAPTION);
			else if (lShift)
				onIncrementWindow(pMsg, -10, IncWnd::All, &pt, HTCAPTION);
			else
				onShowPosWnd(pMsg, pt);
		}
	}
}

/**
 * @brief method to change the size of a window in steps
 * @param pMsg MSG structure from hook
 * @param diff step-size
 * @param incDir direction to grow or shrink the window
 * @param pCursorPos position of cursor, if displacement is desired 
 * @param hitTest information, where the click has hit
*/
void HooksImpl::onIncrementWindow(MSG* pMsg, int diff, IncWnd incDir /*= IncWnd::All*/, POINT* pCursorPos/*=nullptr*/, LRESULT hitTest /*= HTNOWHERE*/, bool async /*= true*/)
{
	HWND hWnd = pMsg->hwnd;

	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);
	if (wp.showCmd == SW_MAXIMIZE && diff > 0)
	{
		WRITE_DEBUG_LOG_DETAIL(format("Msg: {:#010x}, diff: {}, dir: {:#06x}", pMsg->message, diff, (unsigned char)incDir),
			format("CursorPos: {}", pMsg->message,pCursorPos ? format("x: {}, y: {}", pCursorPos->x, pCursorPos->y) : string("NULL")));
		return;
	}

	wp.flags = WPF_ASYNCWINDOWPLACEMENT;

	RECT wr1 = { 0 };// wp.rcNormalPosition;
	GetWindowRect(hWnd, &wr1);
	RECT wr2 = wr1;

	if (check_one_bit(IncWnd::Left, incDir))
		wr2.left -= diff;
	if (check_one_bit<uint8_t>(IncWnd::Right, incDir))
		wr2.right += diff;
	if (check_one_bit<uint8_t>(IncWnd::Up, incDir))
		wr2.top -= diff;
	if (check_one_bit<uint8_t>(IncWnd::Down, incDir))
		wr2.bottom += diff;
	//InflateRect(&wr, diff, diff);

	WRITE_DEBUG_LOG_DETAIL(format("Msg: {:#010x}, diff: {}, dir: {:#06x}", pMsg->message, diff, (unsigned char)incDir),
		format("CursorPos: {}, RECT1: {},{},{},{}, RECT2: {},{},{},{}", 
			pCursorPos ? format("x: {}, y: {}", pCursorPos->x, pCursorPos->y) : string("NULL"),
		wr1.left, wr1.top, wr1.right, wr1.bottom,
		wr2.left, wr2.top, wr2.right, wr2.bottom)
	);

	//auto width = r.right - r.left;
	auto height = wr2.bottom - wr2.top;

	auto captionHeight = GetSystemMetrics(SM_CYCAPTION);
	if (height <= captionHeight)
		return;

	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(MONITORINFO) };
	GetMonitorInfo(hMon, &mi);
	auto mr = mi.rcWork;
	if (height >= (mr.bottom - mr.top))
	{
		wp.showCmd = SW_MAXIMIZE;
		SetWindowPlacement(hWnd, &wp);
		return;
	}

	if (pCursorPos != nullptr)
	{
		POINT& pt = *pCursorPos;
		if (check_one_bit<uint8_t>(IncWnd::Up, incDir) && hitTest == HTCAPTION)
			SetCursorPos(pt.x, pt.y - diff); // keep cursor over caption
		if (check_one_bit<uint8_t>(IncWnd::Up, incDir) && hitTest == HTTOP)
			SetCursorPos(pt.x , pt.y - diff); // keep cursor over top border;
		if (check_one_bit<uint8_t>(IncWnd::Left, incDir) && hitTest == HTLEFT)
			SetCursorPos(pt.x - diff, pt.y); // keep cursor over left border
		if (check_one_bit<uint8_t>(IncWnd::Right, incDir) && hitTest == HTRIGHT)
			SetCursorPos(pt.x + diff, pt.y); // keep cursor over right border;
		if (check_one_bit<uint8_t>(IncWnd::Down, incDir) && hitTest == HTBOTTOM)
			SetCursorPos(pt.x, pt.y + diff); // keep cursor over bottom border;
	}

	wp.rcNormalPosition = wr2;
	wp.showCmd = SW_NORMAL;
	if(async)
	{
		// async, because the behavior when clicking on the border is otherwise strange
		auto resFuture = std::async(std::launch::async, [hWnd, wp]()
			{
				BOOL wndReplaced = SetWindowPlacement(hWnd, &wp);
				assert(wndReplaced);
			});
	}
	else
	{
		wp.flags = 0;
		BOOL wndReplaced = SetWindowPlacement(hWnd, &wp);
		assert(wndReplaced);
	}
}

/**
 * @brief 
 * @param pMsg 
 * @param pt 
*/
void HooksImpl::onClosePosWnd(MSG* pMsg, POINT pt)
{
	if (WinPosWnd* winPosWnd = _winPosWnd.getWinPosWnd(pMsg ? pMsg->hwnd : NULL))
		return;		
	_winPosWnd.destroy();
}

/**
 * @brief 
 * @param pMsg 
 * @param pt 
*/
HWND HooksImpl::onShowPosWnd(MSG* pMsg, POINT pt)
{
	_winPosWnd.create(pt, pMsg->hwnd);
	return _winPosWnd.getWndHandle();
}
