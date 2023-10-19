#include "pch.h"
#include "HooksMgr.h"
#include <cassert>
#include "Utils.h"

/**
 * @brief 
 * @return 
*/
HMODULE HooksMgr::load()
{
	HMODULE hModule = NULL;
#ifdef _WIN64
	hModule = LoadLibrary(L"wiNpoS-Hook64.dll");
#else
	hModule = LoadLibrary(L"wiNpoS-Hook32.dll");
#endif // _WIN64
	return hModule;
}

/**
 * @brief 
 * @param hModule 
*/
void HooksMgr::unloadHook(HMODULE hModule)
{
	BOOL libraryFreed = FreeLibrary(hModule);
	assert(libraryFreed);
}

/**
 * @brief 
*/
void HooksMgr::loadHook()
{
	_hModule = load();
}

/**
 * @brief 
*/
void HooksMgr::unloadHook()
{
	unloadHook(_hModule);
}

/**
 * @brief 
*/
void HooksMgr::attach()
{
	_hModule = load();
	setHooks(_hModule);
}

/**
 * @brief 
 * @param hModule 
*/
void HooksMgr::setHooks(HMODULE hModule)
{
	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(hModule, STRINGIZE(CallWndProc));
	assert(hkCallWndProc);

	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(hModule, STRINGIZE(GetMsgProc));
	assert(hkGetMsgProc);

	_hhkCallWndProc = SetWindowsHookEx(WH_CALLWNDPROC, hkCallWndProc, NULL, GetCurrentThreadId());
	assert(_hhkCallWndProc);

	_hhkGetMessage = SetWindowsHookEx(WH_GETMESSAGE, hkGetMsgProc, NULL, GetCurrentThreadId());
	assert(_hhkGetMessage);
}

/**
 * @brief 
*/
void HooksMgr::detach()
{
	unhookHooks();

	unloadHook(_hModule);
}

/**
 * @brief 
*/
void HooksMgr::unhookHooks()
{
	if (_hhkGetMessage && UnhookWindowsHookEx(_hhkGetMessage))
		_hhkGetMessage = NULL;
	assert(_hhkGetMessage == NULL);

	if (_hhkCallWndProc && UnhookWindowsHookEx(_hhkCallWndProc))
		_hhkCallWndProc = NULL;
	assert(_hhkCallWndProc == NULL);

	//if (_hhkShellHookProc && UnhookWindowsHookEx(_hhkShellHookProc))
	//	_hhkShellHookProc = NULL;
	//assert(_hhkShellHookProc == NULL);
}

/**
 * @brief 
*/
void HooksMgr::install()
{
	_hModule = load();

	//HOOKPROC hkShellHookProc = (HOOKPROC)GetProcAddress(_hModule, STRINGIZE(ShellHookProc));
	//assert(hkShellHookProc);

	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(_hModule, STRINGIZE(CallWndProc));
	assert(hkCallWndProc);

	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(_hModule, STRINGIZE(GetMsgProc));
	assert(hkGetMsgProc);

	//_hhkShellHookProc = SetWindowsHookEx(WH_SHELL, hkShellHookProc, _hModule, NULL);
	//assert(_hhkShellHookProc);

	_hhkCallWndProc = SetWindowsHookEx(WH_CALLWNDPROC, hkCallWndProc, _hModule, NULL);
	assert(_hhkCallWndProc);

	_hhkGetMessage = SetWindowsHookEx(WH_GETMESSAGE, hkGetMsgProc, _hModule, NULL);
	assert(_hhkGetMessage);
}

/**
 * @brief 
*/
void HooksMgr::uninstall()
{
	unhookHooks();
	//PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, NULL, NULL);
	//detach();
}
