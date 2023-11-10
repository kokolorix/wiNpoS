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
namespace
{
	using HookMutex = std::mutex;
	using HookLock = std::lock_guard<HookMutex>;
	using HookMap = std::map<DWORD, HHOOK>;
	HookMutex hookMutex;
	HookMap hookMap;

	HooksMgrPtr hooksMgr = std::make_unique<HooksMgr>(); ///> the hooks manager


	std::unique_ptr<std::latch> allHookedThreads;

	LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	void UnhookThreadHook();

	thread_local HHOOK hhShellHookProc = NULL;
	LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			Utils::DllName = Utils::getDllName(hModule);
			hInstance = hModule;
			
			hooksMgr->loadHook();

			using std::ranges::views::filter;
			using std::ranges::to;
			std::set<DWORD> threadIds;
			auto mainWnds = GetMainWnds(0, GetCurrentProcessId()) | filter([&threadIds](const MainWndRes& mw) { return threadIds.insert(mw.wndThreadId).second; }) | to<MainWindResVector>();
			allHookedThreads = std::make_unique<std::latch>(mainWnds.size());
			std::thread t(
				[mainWnds]() 
				{
					for (MainWndRes mw : mainWnds)
					{
						HookLock lock(hookMutex);
						hookMap[mw.wndThreadId] =
							SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);
						PostMessage(mw.hMainWnd, MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK, (WPARAM)0, (LPARAM)0);
						PostMessage(mw.hMainWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
					}
					allHookedThreads->wait();
					WRITE_DEBUG_LOG("All root windows hooked");
				});
			t.detach();

			WRITE_DEBUG_LOG(format("Attach {} to {}", Utils::getDllName(hInstance), Utils::ExeName));
			break;
		}
		break;
		case DLL_THREAD_ATTACH:
		{
			hhShellHookProc =
				SetWindowsHookEx(WH_SHELL, ShellHookProc, NULL, GetCurrentThreadId());
			break;
		}
		case DLL_THREAD_DETACH:
		{
			UnhookThreadHook();
			if(hhShellHookProc)
				UnhookWindowsHookEx(hhShellHookProc);
			break;
		}
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

namespace
{
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
						HookLock lock(hookMutex);
						hookMap[GetCurrentThreadId()] =
							SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, GetCurrentThreadId());
					}

					else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK)
					{
						WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						
						UnhookThreadHook();

						HWND hSrcWnd = (HWND)pMsg->wParam;
						PostMessage(hSrcWnd, MT_HOOK_MSG_SUPPORT_THREAD_HOOK_UNREGISTERED, (WPARAM)pMsg->hwnd, (LPARAM)hInstance);
					}

					else if (pMsg->message == MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK)
					{
						WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_REGISTER_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						if (!hooksMgr->areHooksSet())
						{
							hooksMgr->setHooks();
							allHookedThreads->count_down();
						}
					}

					else if (pMsg->message == MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK)
					{
						WRITE_DEBUG_LOG(dformat("MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK: {:#010x}, hWnd: {:018x} ", pMsg->message, (uint64_t)pMsg->hwnd));
						if (hooksMgr->areHooksSet())
						{
							hooksMgr->unhookHooks();
							allHookedThreads->count_down();
						}
					}
				}
			}
		}

		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}

	void UnhookThreadHook()
	{
		HHOOK hHook = NULL;
		{
			HookLock lock(hookMutex);
			auto it = hookMap.find(GetCurrentThreadId());
			if (it != hookMap.end())
			{
				hHook = it->second;
				hookMap.erase(it);
			}
		}
		if (hHook)
		{
			UnhookWindowsHookEx(hHook);
		}
	}

	LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HSHELL_WINDOWCREATED)
		{
			HWND hNewWindow = (HWND)wParam;
			if (!hooksMgr->areHooksSet())
			{
				hooksMgr->setHooks();
				PostMessage(hNewWindow, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}

}


