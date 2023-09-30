// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Utils.h"
#include "HooksMgr.h"
#include <assert.h>

HINSTANCE hInstance = NULL;   // current instance

thread_local HHOOK hhGetMessageHookProc = NULL;
bool hookThreadInstalled = false;

LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	switch (nCode)
	{
		case HC_ACTION:
		{
			if (wParam == PM_REMOVE || wParam == PM_NOREMOVE || wParam == PM_NOYIELD)
			{
				MSG* pMsg = (MSG*)lParam;

				//WRITE_DEBUG_LOG(format("Msg: {}", pMsg->message));

				if (pMsg->message == MT_HOOK_MSG_REGISTER_SUPPORT_THREAD_HOOK)
				{
					WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_REGISTER_SUPPORT_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
					hhGetMessageHookProc =
						SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, GetCurrentThreadId());
					hookThreadInstalled = true;
				}

				else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK)
				{
					WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
					if (hhGetMessageHookProc && UnhookWindowsHookEx(hhGetMessageHookProc))
						hhGetMessageHookProc = NULL;
					assert(hhGetMessageHookProc == NULL);
					hookThreadInstalled = false;
					HWND hSrcWnd = (HWND)pMsg->wParam;
					PostMessage(hSrcWnd, MT_HOOK_MSG_SUPPORT_THREAD_HOOK_UNREGISTERED, (WPARAM)pMsg->hwnd, (LPARAM)hInstance);
				}

				//else if (pMsg->message == MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK)
				//{
				//	WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_REGISTER_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
				//	hooks.setHooks(hInstance);
				//}

				//else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK)
				//{
				//	WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
				//	hooks.unhookHooks();
				//}

				//else if (pMsg->message == MT_HOOK_MSG_CREATE_TASK_TOOLBAR)
				//{
				//	WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_CREATE_TASK_TOOLBAR: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));

				//	HWND hMainWnd = pMsg->hwnd; // GetAncestor(pMsg->hwnd, GA_ROOT);

				//	HRESULT hr = thumbnailToolbar.initialize(hInstance, hMainWnd);
				//	if (hr == 0x800401f0)//0x800401f0 : CoInitialize has not been called.
				//	{
				//		coInit = CoInitialize(NULL);
				//		hr = thumbnailToolbar.initialize(hInstance, hMainWnd);
				//	}
				//}

				//else if (pMsg->message == MT_HOOK_MSG_DESTROY_TASK_TOOLBAR)
				//{
				//	WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_DESTROY_TASK_TOOLBAR: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
				//	HWND hMainWnd = pMsg->hwnd; // GetAncestor(pMsg->hwnd, GA_ROOT);
				//	thumbnailToolbar.uninitialize(hInstance, hMainWnd);
				//}

				//else if (pMsg->message == MT_HOOK_MSG_UNLOAD)
				//{
				//	WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNLOAD: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));

				//	if (pMsg->wParam != GetCurrentProcessId())
				//	{
				//		hooks.unhookHooks();

				//		if (hhGetMessageHookProc && UnhookWindowsHookEx(hhGetMessageHookProc))
				//			hhGetMessageHookProc = NULL;
				//		assert(hhGetMessageHookProc == NULL);

				//		{
				//			//WRITE_DEBUG_LOG(format("Free {:#018x}", (uint64_t)hInstance));
				//			FreeLibrary(hInstance);
				//		}
				//	}
				//}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
	MainWndRes mw = GetMainWnd();
	hhGetMessageHookProc =
		SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);
	PostMessage(mw.hMainWnd, MT_HOOK_MSG_REGISTER_SUPPORT_THREAD_HOOK, 0, 0);
	while (!hookThreadInstalled) // wait for GetMessageHookProc is installed
		Sleep(200);
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

