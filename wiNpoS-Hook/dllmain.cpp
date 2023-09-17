/ dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <vector>
#include <Utils.h>
#include "HooksMgr.h"
#include <Shlwapi.h>
#include <assert.h>
#include "ThumbnailToolbar.h"
#pragma comment(lib, "Shlwapi.lib")

HINSTANCE hInstance = NULL;   // current instance
HHOOK hhGetMessageHookProc = NULL;
ThumbnailToolbar thumbnailToolbar;
thread_local HHOOK hhShellHookProc = NULL;

struct GetMainWndRes
{
	HWND hMainWnd;
	DWORD wndThreadId;

	operator HWND () const { return hMainWnd; }
	operator bool() const { return hMainWnd != NULL; }
};
using WndVector = std::vector<HWND>;

GetMainWndRes GetMainWnd(DWORD currentThreadId = 0);
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
			hInstance = hModule;
			GetMainWndRes mw = GetMainWnd(0);
			if (mw)
			{
				hhGetMessageHookProc =
					SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);

				thumbnailToolbar.initialize(hInstance, mw);

				char filePath[MAX_PATH] = { 0 };
				GetModuleFileNameA(hInstance, filePath, MAX_PATH);
				string dllName = PathFindFileNameA(filePath);
				GetModuleFileNameA(NULL, filePath, MAX_PATH);
				string exeName = PathFindFileNameA(filePath);
				WRITE_DEBUG_LOG(format("Attach {} to {}", dllName, exeName));
			}
			//else
			//{
			  // FreeLibrary(hModule);
			//}

			break;
		}

		case DLL_THREAD_ATTACH:
			CoInitialize(NULL);
			hhShellHookProc = SetWindowsHookEx(WH_SHELL, ShellHookProc, NULL, GetCurrentThreadId());
			break;
		case DLL_THREAD_DETACH:
			if (UnhookWindowsHookEx(hhShellHookProc))
				hhShellHookProc = NULL;
			assert(hhShellHookProc == NULL);
			CoUninitialize();
			break;

		case DLL_PROCESS_DETACH:
		{
			if (UnhookWindowsHookEx(hhGetMessageHookProc))
				hhGetMessageHookProc = NULL;
			assert(hhGetMessageHookProc == NULL);

			char filePath[MAX_PATH] = { 0 };
			GetModuleFileNameA(hInstance, filePath, MAX_PATH);
			string dllName = PathFindFileNameA(filePath);
			GetModuleFileNameA(NULL, filePath, MAX_PATH);
			string exeName = PathFindFileNameA(filePath);
			WRITE_DEBUG_LOG(format("Detach {} from {}", dllName, exeName));
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
				if (pMsg->message == MT_HOOK_MSG_UNLOAD)
				{
					char filePath[MAX_PATH] = { 0 };
					GetModuleFileNameA(hInstance, filePath, MAX_PATH);
					string dllName = PathFindFileNameA(filePath);
					GetModuleFileNameA(NULL, filePath, MAX_PATH);
					string exeName = PathFindFileNameA(filePath);

					WRITE_DEBUG_LOG(format("Msg: {}(MT_HOOK_MSG_UNLOAD) in {}", pMsg->message, exeName));

					if(pMsg->wParam != GetCurrentProcessId())
					{
						std::thread t([dllName, exeName]() {
							WRITE_DEBUG_LOG(format("Unload {} from {}", dllName, exeName));
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

WndVector GetDesktopWnds();

GetMainWndRes GetMainWnd(DWORD currentThreadId /*= 0*/)
{
	auto wnds = GetDesktopWnds();

	auto currentProcessId = GetCurrentProcessId();

	for (HWND hWnd : wnds) 
	{
		DWORD wndProcessId = NULL;
		auto wndThreadId = GetWindowThreadProcessId(hWnd, &wndProcessId);
		if (currentProcessId == wndProcessId && (currentThreadId == 0 || wndThreadId == currentThreadId))
		{
			HWND hMainWnd = GetAncestor(hWnd, GA_ROOT);
			GetMainWndRes res = { hMainWnd, wndThreadId };
			return res;
		}
	}

	GetMainWndRes res = { 0 };
	return res;
}

namespace
{
	BOOL CALLBACK EnumWindowsProc(HWND   hWnd, LPARAM lParam)
	{
		WndVector& hWnds = *reinterpret_cast<WndVector*>(lParam);
		hWnds.push_back(hWnd);
		return TRUE;
	}
}

WndVector GetDesktopWnds()
{
	WndVector hWnds;
	auto threadId = GetCurrentThreadId();
	auto hDesktop = GetThreadDesktop(threadId);

	auto res = EnumDesktopWindows(hDesktop, EnumWindowsProc, (LPARAM)&hWnds);

	return hWnds;	//EnumDesktopWindows()
}

LRESULT CALLBACK ShellHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD currentThreadId = GetCurrentThreadId();

	if (nCode == HSHELL_WINDOWCREATED)
	{
		HWND hNewWindow = (HWND)wParam;
		thumbnailToolbar.initialize(hInstance, hNewWindow);
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

