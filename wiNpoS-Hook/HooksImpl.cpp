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

			char filePath[MAX_PATH] = { 0 };
			GetModuleFileNameA(NULL, filePath, MAX_PATH);
			string exeName = PathFindFileNameA(filePath);
			WRITE_DEBUG_LOG(format("Msg: {} in {}", sMsg->message, exeName));
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

				char filePath[MAX_PATH] = { 0 };
				GetModuleFileNameA(NULL, filePath, MAX_PATH);
				string exeName = PathFindFileNameA(filePath);
				WRITE_DEBUG_LOG(format("Msg: {} in {}", pMsg->message, exeName));
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
