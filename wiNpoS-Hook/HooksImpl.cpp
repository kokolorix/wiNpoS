#include "pch.h"
#include "HooksImpl.h"
#include "HooksMgr.h"
#include "Utils.h"

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
			//WRITE_DEBUG_LOG(format("Msg: {}", sMsg->message));
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
				//WRITE_DEBUG_LOG(format("Msg: {}, UNLOAD: {}", pMsg->message, MT_HOOK_MSG_UNLOAD));
				//if (pMsg->message == MT_HOOK_MSG_UNLOAD)
				//{
				//	if (hInstance)
				//		HooksMgr::unload(hInstance);
				//}
				//switch (pMsg->message)
				//{
				//	case MT_HOOK_MSG_UNLOAD:
				//	default:
				//		break;
				//}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
