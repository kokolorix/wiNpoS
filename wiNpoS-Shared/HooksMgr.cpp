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
void HooksMgr::unload(HMODULE hModule)
{
	assert(FreeLibrary(hModule));
}

/**
 * @brief 
*/
void HooksMgr::attach()
{
	_hModule = load();
}
/**
 * @brief 
*/
void HooksMgr::detach()
{
	unload(_hModule);
}
/**
 * @brief 
*/
void HooksMgr::install()
{
	attach();

	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(_hModule, STRINGIZE(CallWndProc));
	assert(hkCallWndProc);

	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(_hModule, STRINGIZE(GetMsgProc));
	assert(hkGetMsgProc);

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
	if (UnhookWindowsHookEx(_hhkCallWndProc))
		_hhkCallWndProc = NULL;
	assert(_hhkCallWndProc == NULL);

	if (UnhookWindowsHookEx(_hhkGetMessage))
		_hhkGetMessage = NULL;
	assert(_hhkGetMessage == NULL);

	//PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, NULL, NULL);
	//detach();
}
