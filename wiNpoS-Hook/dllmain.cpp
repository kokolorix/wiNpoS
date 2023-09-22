// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <vector>
#include <Utils.h>
#include "HooksMgr.h"
#include <Shlwapi.h>
#include <assert.h>
#include "ThumbnailToolbar.h"
#pragma comment(lib, "Shlwapi.lib")

HINSTANCE hInstance = NULL;   // current instance
ThumbnailToolbar thumbnailToolbar;
thread_local HooksMgr hooks;
thread_local HHOOK hhGetMessageHookProc = NULL;
thread_local HHOOK hhShellHookProc = NULL;
thread_local HRESULT coInit = S_FALSE;

LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch ((int)ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			//while (true) {
			//	Sleep(500);
			//}
			hInstance = hModule;
			GetMainWndRes mw = GetMainWnd(); // for current process and thread
			if (mw) // only apps with main window 
			{

				if (mw.wndThreadId == GetCurrentThreadId()) // its a regular attach from global hook
				{
					hhGetMessageHookProc =
						SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);

					PostMessage(mw.hMainWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)GetCurrentThreadId(), (LPARAM)GetCurrentThread());
				}
				else	// otherwise its a attach by infiltrated thread
				{		// here we must start a new thread, to release the calling process

					std::thread t([mw]()
						{	// this hook allows the continuation in the main thread of the application
							hhGetMessageHookProc =
								SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);

							PostMessage(mw.hMainWnd, MT_HOOK_MSG_REGISTER_THREAD_HOOK, 0, 0);
							//PostMessage(mw.hMainWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)GetCurrentThreadId(), (LPARAM)GetCurrentThread());
							PostMessage(mw.hMainWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, 0, 0);

							MSG msg;
							while (GetMessage(&msg, NULL, 0, 0))
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
						});

					t.detach(); // because the end of control is reached here
				}

				WRITE_DEBUG_LOG(format("Attach {} to {}", Utils::DllName, Utils::ExeName));
			}

			break;
		}

		case DLL_THREAD_ATTACH:
			hhShellHookProc = 
				SetWindowsHookEx(WH_SHELL, ShellHookProc, NULL, GetCurrentThreadId());
			break;

		case DLL_THREAD_DETACH:
			if (UnhookWindowsHookEx(hhShellHookProc))
				hhShellHookProc = NULL;
			assert(hhShellHookProc == NULL);

			hooks.unhookHooks();

			if(coInit == S_OK)
				CoUninitialize();

			break;

		case DLL_PROCESS_DETACH:
		{
			if (UnhookWindowsHookEx(hhGetMessageHookProc))
				hhGetMessageHookProc = NULL;
			assert(hhGetMessageHookProc == NULL);

			WRITE_DEBUG_LOG(format("Detach {} from {}", Utils::DllName, Utils::ExeName));
			break;
		}
	}
	return TRUE;
}

LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	switch (nCode)
	{
		case HC_ACTION:
		{
			if (wParam == PM_REMOVE)
			{
				MSG* pMsg = (MSG*)lParam;

				//WRITE_DEBUG_LOG(format("Msg: {}", pMsg->message));

				if (pMsg->message == MT_HOOK_MSG_REGISTER_THREAD_HOOK)
				{
					hooks.setHooks(hInstance);
				}

				else if (pMsg->message == MT_HOOK_MSG_CREATE_TASK_TOOLBAR)
				{
					HRESULT hr = thumbnailToolbar.initialize(hInstance, pMsg->hwnd);
					if (hr == 0x800401f0)//0x800401f0 : CoInitialize has not been called.
					{
						coInit = CoInitialize(NULL);
						hr = thumbnailToolbar.initialize(hInstance, pMsg->hwnd);
					}
					if (FAILED(hr))
						WRITE_DEBUG_LOG(format("Initialize of task toolbar failed with {:#018x} for {}", hr, Utils::ExeName));
					else
						WRITE_DEBUG_LOG(format("Initialize of task toolbar succede with {:#018x} for {}", hr, Utils::ExeName));

					uint32_t sourceThreadId = (uint32_t)pMsg->wParam;
					if(sourceThreadId != GetCurrentThreadId())
						PostThreadMessage(sourceThreadId, WM_QUIT, 0, 0);
				}

				else if (pMsg->message == MT_HOOK_MSG_UNLOAD)
				{
					WRITE_DEBUG_LOG(format("Msg: {}(MT_HOOK_MSG_UNLOAD) in {}", pMsg->message, Utils::ExeName));

					if(pMsg->wParam != GetCurrentProcessId())
					{
						std::thread t([]() {
							WRITE_DEBUG_LOG(format("Unload {} from {}", Utils::DllName, Utils::ExeName));
							FreeLibrary(hInstance);
							});

						t.detach(); // because the end of control is reached here
					}
				}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD currentThreadId = GetCurrentThreadId();

	if (nCode == HSHELL_WINDOWCREATED)
	{
		HWND hNewWindow = (HWND)wParam;
		hhGetMessageHookProc =
			SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, GetCurrentThreadId());

		PostMessage(hNewWindow, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)GetCurrentThreadId(), (LPARAM)GetCurrentThread());
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

