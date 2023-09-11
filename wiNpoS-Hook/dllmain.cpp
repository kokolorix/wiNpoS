// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <vector>
#include <Utils.h>
#include "HooksMgr.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

HINSTANCE hInstance = NULL;   // current instance
HHOOK hhGetMessageHookProc = NULL;

struct GetMainWndRes
{
	HWND hMainWnd;
	DWORD wndThreadId;
	operator HWND () const { return hMainWnd; }
	operator bool() const { return hMainWnd != NULL; }
};
GetMainWndRes GetMainWnd(DWORD currentThreadId = 0);
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
		 hInstance = hModule;
		 GetMainWndRes mw = GetMainWnd(0);
		 if (mw)
		 {
			 hhGetMessageHookProc =
				 SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, mw.wndThreadId);

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
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
	 {		
		 if (hhGetMessageHookProc)
			 UnhookWindowsHookEx(hhGetMessageHookProc);

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

using WndVector = std::vector<HWND>;
WndVector GetDesktopWnds();

GetMainWndRes GetMainWnd(DWORD currentThreadId /*= 0*/)
{
	auto wnds = GetDesktopWnds();

	auto currentProcessId = GetCurrentProcessId();

	for (HWND hWnd : wnds) {
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

BOOL CALLBACK EnumWindowsProc(HWND   hWnd, LPARAM lParam)
{
	WndVector& hWnds = *reinterpret_cast<WndVector*>(lParam);
	hWnds.push_back(hWnd);
	return TRUE;
}

WndVector GetDesktopWnds()
{
	WndVector hWnds;
	auto threadId = GetCurrentThreadId();
	auto hDesktop = GetThreadDesktop(threadId);

	auto res = EnumDesktopWindows(hDesktop, EnumWindowsProc, (LPARAM)&hWnds);

	return hWnds;	//EnumDesktopWindows()
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
					WRITE_DEBUG_LOG(format("Unload {} from {}", dllName, exeName));
				}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
