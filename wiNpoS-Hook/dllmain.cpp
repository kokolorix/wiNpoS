// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <vector>
//#include <condition_variable>
#include <Utils.h>
#include "HooksMgr.h"
#include <Shlwapi.h>
#include <assert.h>
#include "TaskToolbar.h"
#pragma comment(lib, "Shlwapi.lib")

HINSTANCE hInstance = NULL;   // current instance

thread_local HooksMgr hooks;
thread_local HHOOK hhShellHookProc = NULL;

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
			hInstance = hModule;
			MainWindResVector mainWnds = GetMainWnds(0, GetCurrentProcessId());
			for (MainWndRes mainWnd : mainWnds)
			{
				PostMessage(mainWnd.hMainWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
			}

			hhShellHookProc =
				SetWindowsHookEx(WH_SHELL, ShellHookProc, NULL, GetCurrentThreadId());

			WRITE_DEBUG_LOG(format("Attach {} to {}", Utils::DllName, Utils::ExeName));

			break;
		}

		case DLL_THREAD_ATTACH:
			hhShellHookProc =
				SetWindowsHookEx(WH_SHELL, ShellHookProc, NULL, GetCurrentThreadId());
			break;

		case DLL_THREAD_DETACH:
			if (hhShellHookProc && UnhookWindowsHookEx(hhShellHookProc))
				hhShellHookProc = NULL;
			assert(hhShellHookProc == NULL);
			break;

		case DLL_PROCESS_DETACH:
		{
			if (hhShellHookProc && UnhookWindowsHookEx(hhShellHookProc))
				hhShellHookProc = NULL;
			assert(hhShellHookProc == NULL);

			WRITE_DEBUG_LOG(format("Detach {} from {}", Utils::DllName, Utils::ExeName));
			break;
		}
		default:
			break;
	}
	return TRUE;
}
