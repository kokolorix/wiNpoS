#include "pch.h"
#include "HooksImpl.h"
#include "HooksMgr.h"
#include "Utils.h"
#include <Shlwapi.h>

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
