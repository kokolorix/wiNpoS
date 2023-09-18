#include "pch.h"
#include "HooksImpl.h"
#include "HooksMgr.h"
#include "Utils.h"
#include <Shlwapi.h>
#include <windowsx.h>
#include <resource.h>

extern HINSTANCE hInstance;

LRESULT CALLBACK CallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::callWndProc(nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::getMsgProc(nCode, wParam, lParam);

}

namespace 
{
	RECT lr = { 0 }; // last rect
	bool hasLastRect()
	{
		return lr.left + lr.right + lr.right + lr.bottom != 0;
	}
	POINT cd = { 0 }; // caption dblclicked
	bool hasCaptionDblClicked()
	{
		return cd.x + cd.y != 0;
	}
}
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
					onWindowPosChanged(pMsg);
					break;
				default:
					break;
			}
			//WRITE_DEBUG_LOG(format("Msg: {} in {}", pMsg->message, Utils::ExeName));
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

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
						WRITE_DEBUG_LOG(format("WM_SHOWWINDOW: {:#010x} ", pMsg->message));
						break;
					case WM_GETMINMAXINFO:
						WRITE_DEBUG_LOG(format("WM_GETMINMAXINFO: {:#010x} ", pMsg->message));
						break;
					case WM_WINDOWPOSCHANGING:
						WRITE_DEBUG_LOG(format("WM_WINDOWPOSCHANGING: {:#010x} ", pMsg->message));
						break;
					case WM_LBUTTONDBLCLK:
						WRITE_DEBUG_LOG(format("WM_LBUTTONDBLCLK: {:#010x} ", pMsg->message));
						break;
					case WM_NCRBUTTONDOWN:
						WRITE_DEBUG_LOG(format("WM_NCRBUTTONDOWN: {:#010x} ", pMsg->message));
						break;
					case WM_NCRBUTTONUP:
						WRITE_DEBUG_LOG(format("WM_NCRBUTTONUP: {:#010x} ", pMsg->message));
						break;
					case WM_NCLBUTTONDOWN:
						WRITE_DEBUG_LOG(format("WM_NCLBUTTONDOWN: {:#010x} ", pMsg->message));
						break;
					case WM_NCLBUTTONUP:
						WRITE_DEBUG_LOG(format("WM_NCLBUTTONUP: {:#010x} ", pMsg->message));
						break;
					case WM_NCLBUTTONDBLCLK:
						WRITE_DEBUG_LOG(format("WM_NCLBUTTONDBLCLK: {:#010x} ", pMsg->message));
						onNcButtonDblClick(pMsg);
						break;
					case WM_COMMAND:
					{
						uint16_t type = HIWORD(pMsg->wParam);
						uint16_t wmId = LOWORD(pMsg->wParam);
						WRITE_DEBUG_LOG(format("WM_COMMAND: {:#010x} ID: {}", pMsg->message, wmId));
						//if (type == Type::Menu)
						//{
							switch (wmId)
							{
								case ID_SYSMENU_INCREMENTWINDOWSIZE:
									onIncrementWindow(pMsg, 10);
									break;
								case ID_SYSMENU_DECREMENTWINDOWSIZE:
									onIncrementWindow(pMsg, -10);
									break;
								default:
									break;
							}
						//}
					}
						break;
					default:
						break;
				}

				//WRITE_DEBUG_LOG(format("Msg: {} in {}", pMsg->message, Utils::ExeName));
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void HooksImpl::onNcButtonDblClick(MSG* pMsg)
{
	//int x = LOWORD(pMsg->lParam);
	//int y = HIWORD(pMsg->lParam);
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
	if (pt.x < cr.left || pt.x >= cr.right)
	{
		if (hasLastRect())
		{
			WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#010x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, SWP_NOZORDER));
			SetWindowPos(pMsg->hwnd, NULL, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, SWP_NOZORDER);
			lr = { 0 };
		}
		else
		{
			// Get the handle to the primary monitor
			HMONITOR hMonitor = MonitorFromWindow(pMsg->hwnd, MONITOR_DEFAULTTOPRIMARY);

			// Get the monitor information
			MONITORINFOEX mi;
			mi.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(hMonitor, &mi);

			// Calculate the width of the monitor
			int ml = mi.rcMonitor.left;
			int mw = mi.rcMonitor.right - mi.rcMonitor.left;
			lr = wr;
			//WINUSERAPI BOOL WINAPI SetWindowPos(_In_ HWND hWnd, _In_opt_ HWND hWndInsertAfter, _In_ int X, _In_ int Y, _In_ int cx, _In_ int cy, _In_ UINT uFlags)
			WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#010x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, ml, wr.top, mw, wr.bottom - wr.top, SWP_NOZORDER));
			SetWindowPos(pMsg->hwnd, NULL, ml, wr.top, mw, wr.bottom - wr.top, SWP_NOZORDER);
		}
	}
	// Check if the double-click occurred in the caption area
	int lowerCaption = wr.top + GetSystemMetrics(SM_CYCAPTION);
	if (pt.y < lowerCaption)
	{
		bool isRelevant =
			hitTest != HTMINBUTTON &&
			hitTest != HTMAXBUTTON &&
			hitTest != HTCLOSE &&
			hitTest != HTSYSMENU;

		if (hasLastRect() && isRelevant)
		{
			cd = pt;
		}
	}
}

void HooksImpl::onWindowPosChanged(CWPSTRUCT* pMsg)
{
	if (hasCaptionDblClicked())
	{
		cd = { 0 };

		if (hasLastRect())
		{
			if (IsZoomed(pMsg->hwnd))
			{
				WRITE_DEBUG_LOG(format("SetWindowPos(hWnd: {:#010x}, hWndInsertAfter: {}, x: {}, y: {}, cx: {}, cy: {}, uFlags: {:#010x})", (int64_t)pMsg->hwnd, NULL, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, SWP_NOZORDER));
				ShowWindow(pMsg->hwnd, SW_NORMAL);
				SetWindowPos(pMsg->hwnd, NULL, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, SWP_NOZORDER);
				lr = { 0 };
			}
		}
	}
}

void HooksImpl::onIncrementWindow(MSG* pMsg, int diff)
{
	HWND hWnd = pMsg->hwnd;

	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);
	if (wp.showCmd == SW_MAXIMIZE && diff > 0)
		return;

	RECT wr = { 0 };
	GetWindowRect(hWnd, &wr);
	InflateRect(&wr, diff, diff);

	//auto width = r.right - r.left;
	auto height = wr.bottom - wr.top;

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

	if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) || HIBYTE(GetAsyncKeyState(VK_SHIFT)))
	{
		POINT pt = { 0 };
		GetCursorPos(&pt);
		//ScreenToClient(hWnd, &pt);
		SetCursorPos(pt.x, pt.y - diff);
	}

	wp.rcNormalPosition = wr;
	wp.showCmd = SW_NORMAL;
	SetWindowPlacement(hWnd, &wp);
	//if(wp.showCmd == SW_MAXIMIZE)
	//ShowWindow(hWnd, SW_NORMAL);
	//int l = 
	//SetWindowPos(hWnd, HWND_NOTOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_SHOWWINDOW);
}
