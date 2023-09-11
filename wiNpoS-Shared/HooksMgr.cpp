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
#ifdef WIN32
	hModule = LoadLibrary(L"wiNpoS-Hook32.dll");
#else
	hModule = LoadLibrary(L"wiNpoS-Hook64.dll");
#endif // Win32
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
	WRITE_DEBUG_LOG(format("Send message {} to all Windows" , MT_HOOK_MSG_UNLOAD));
	assert(PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, NULL, NULL));
	//unload(_hModule);
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
	PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, NULL, NULL);

	if (UnhookWindowsHookEx(_hhkCallWndProc))
		_hhkCallWndProc = NULL;
	assert(_hhkCallWndProc == NULL);

	if (UnhookWindowsHookEx(_hhkGetMessage))
		_hhkGetMessage = NULL;
	assert(_hhkGetMessage == NULL);

	//detach();
}
