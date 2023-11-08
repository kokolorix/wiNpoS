// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Utils.h"
#include "HooksMgr.h"
#include <assert.h>
#include <set>
#include <latch>
#include <future>
#include <ranges>

HINSTANCE hInstance = NULL;   // current instance

thread_local HHOOK hhGetMessageHookProc = NULL;
//bool hookThreadInstalled = false;

HooksMgrPtr _hooksMgr = std::make_unique<HooksMgr>(); ///> the hooks manager

//std::atomic<std::set<DWORD>> installedThreadIds;
std::unique_ptr<std::latch> allHookedThreads;

LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			Utils::DllName = Utils::getDllName(hModule);
			hInstance = hModule;
			
			//HMODULE hHookModule = _hooksMgr->load();
			_hooksMgr->loadHook();

			using std::ranges::views::filter;
			using std::ranges::to;
			std::set<DWORD> threadIds;
			auto mainWnds = GetMainWnds(0, GetCurrentProcessId()) | filter([&threadIds](const MainWndRes& mw) { return threadIds.insert(mw.wndThreadId).second; }) | to<MainWindResVector>();
			allHookedThreads = std::make_unique<std::latch>(mainWnds.size());
			//auto future = std::async(std::launch::async, 
			std::thread t(
				[mainWnds]() 
				{
					for (MainWndRes mw : mainWnds)
					{
						hhGetMessageHookProc =
							SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);
						PostMessage(mw.hMainWnd, MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK, 0, 0);
					}
					allHookedThreads->wait();
					WRITE_DEBUG_LOG("All root windows hooked");
				});
			//future.wait();
			t.detach();

			WRITE_DEBUG_LOG(format("Attach {} to {}", Utils::getDllName(hInstance), Utils::ExeName));
		}
		break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

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
				}

				else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK)
				{
					WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
					if (hhGetMessageHookProc && UnhookWindowsHookEx(hhGetMessageHookProc))
						hhGetMessageHookProc = NULL;
					assert(hhGetMessageHookProc == NULL);

					HWND hSrcWnd = (HWND)pMsg->wParam;
					PostMessage(hSrcWnd, MT_HOOK_MSG_SUPPORT_THREAD_HOOK_UNREGISTERED, (WPARAM)pMsg->hwnd, (LPARAM)hInstance);
				}

				else if (pMsg->message == MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK)
				{
					WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_REGISTER_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
					_hooksMgr->setHooks();
					allHookedThreads->count_down();
				}

				else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK)
				{
					WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
					_hooksMgr->unhookHooks();
					allHookedThreads->count_down();
				}

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

