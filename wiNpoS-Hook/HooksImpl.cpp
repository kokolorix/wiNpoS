#include "pch.h"
#include "HooksImpl.h"
#include "HooksMgr.h"
#include "Utils.h"
#include <Shlwapi.h>
#include <windowsx.h>

extern HINSTANCE hInstance;

LRESULT CALLBACK CallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::callWndProc(nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return HooksImpl::getMsgProc(nCode, wParam, lParam);

}

LRESULT CALLBACK HooksImpl::callWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (nCode)
	{
		case HC_ACTION:
		{
			CWPSTRUCT* sMsg = (CWPSTRUCT*)lParam;

			WRITE_DEBUG_LOG(format("Msg: {} in {}", sMsg->message, Utils::ExeName));
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

				switch ( pMsg->message)
				{
				//case WM_MOUSEMOVE:
				//	WRITE_DEBUG_LOG(format("WM_MOUSEMOVE: {:#010x} ", pMsg->message));
				//	break;
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
					{
						//int x = LOWORD(lParam);
						//int y = HIWORD(lParam);
						int x = GET_X_LPARAM(lParam);
						int y = GET_Y_LPARAM(lParam);

						// Get the window's client rectangle
						RECT cr;
						GetClientRect(pMsg->hwnd, &cr);

						RECT wr;
						GetWindowRect(pMsg->hwnd, &wr);

						// Adjust the client rectangle to screen coordinates
						MapWindowPoints(pMsg->hwnd, HWND_DESKTOP, (LPPOINT)&cr, 2);

						static RECT lr = { 0 }; // last rect
						// Check if the double-click occurred in the border area
						if (x < cr.left || x >= cr.right)
						{
							if (lr.left + lr.right  + lr.right + lr.bottom == 0 )
							{
								// Get the handle to the primary monitor
								HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);

								// Get the monitor information
								MONITORINFOEX mi;
								mi.cbSize = sizeof(MONITORINFOEX);
								GetMonitorInfo(hMonitor, &mi);

								// Calculate the width of the monitor
								int monitorWidth = mi.rcMonitor.right - mi.rcMonitor.left;
								lr = wr;

								SetWindowPos(pMsg->hwnd, NULL, 0, wr.top, monitorWidth, wr.bottom - wr.top, SWP_NOZORDER);
							}
							else
							{
								SetWindowPos(pMsg->hwnd, NULL, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, SWP_NOZORDER);
								lr = { 0 };
							}
						}
						// Check if the double-click occurred in the caption area
						if (y < (wr.top + GetSystemMetrics(SM_CYCAPTION)))
						{
							if (lr.left + lr.right + lr.right + lr.bottom != 0)
							{
								ShowWindow(pMsg->hwnd, SW_NORMAL);
								SetWindowPos(pMsg->hwnd, NULL, lr.left, lr.top, lr.right  - lr.left, lr.bottom - lr.top, SWP_NOZORDER);
								lr = { 0 };
							}
						}
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
