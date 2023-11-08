#include "pch.h"
#include "DebugNew.h"
#include "HooksMgr.h"
#include <regex>
#include "Utils.h"
#include <Shlwapi.h>

extern HINSTANCE hInstance;   // current instance

/**
 * @brief 
 * @return 
*/
HMODULE HooksMgr::load()
{
	HMODULE hModule = NULL;

	char path[MAX_PATH];
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(hInstance, path, MAX_PATH);
	PathRemoveFileSpecA(path);

#ifdef _WIN64
	PathAppendA(path, "wiNpoS-Hook64.dll");
#else
	PathAppendA(path, "wiNpoS-Hook32.dll");
#endif // _WIN64

	hModule = LoadLibraryA(path);
	return hModule;
}

/**
 * @brief 
 * @param hModule 
*/
void HooksMgr::unloadHook(HMODULE hModule)
{
	BOOL libraryFreed = FreeLibrary(hModule);
	AssertTrue(libraryFreed, "Library not freed");
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
 * @param threadId 
*/
void HooksMgr::setHooks(DWORD threadId /*= GetCurrentThreadId()*/)
{
	setHooks(_hModule, threadId);
}

/**
 * @brief 
 * @param hModule 
*/
void HooksMgr::setHooks(HMODULE hModule, DWORD threadId /*= GetCurrentThreadId()*/)
{
	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(hModule, S(CallWndProc));
	AssertTrue(hkCallWndProc, "CallWndProc adress should not be NULL");

	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(hModule, S(GetMsgProc));
	AssertTrue(hkGetMsgProc, "GetMsgProc adress should not be NULL");

	HookMapLock lock(_mutex);

	HHOOK _hhkCallWndProc = _callWndProcMap[threadId] = SetWindowsHookEx(WH_CALLWNDPROC, hkCallWndProc, NULL, threadId);
	AssertTrue(_hhkCallWndProc, "Hook handle should not be NULL");

	HHOOK _hhkGetMessage = _getMessageMap[threadId] = SetWindowsHookEx(WH_GETMESSAGE, hkGetMsgProc, NULL, threadId);
	AssertTrue(_hhkGetMessage, "Hook handle should not be NULL");
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
void HooksMgr::unhookHooks(DWORD threadId /*= GetCurrentThreadId()*/)
{
	HookMapLock lock(_mutex);

	HHOOK& _hhkGetMessage = _getMessageMap[threadId];
	if (_hhkGetMessage && UnhookWindowsHookEx(_hhkGetMessage))
		_hhkGetMessage = NULL;
	AssertTrue(_hhkGetMessage == NULL, "Hook handle should be NULL");
	_getMessageMap.erase(threadId);

	HHOOK& _hhkCallWndProc = _callWndProcMap[threadId];
	if (_hhkCallWndProc && UnhookWindowsHookEx(_hhkCallWndProc))
		_hhkCallWndProc = NULL;
	AssertTrue(_hhkCallWndProc == NULL, "Hook handle should be NULL");
	_callWndProcMap.erase(threadId);

	//if (_hhkShellHookProc && UnhookWindowsHookEx(_hhkShellHookProc))
	//	_hhkShellHookProc = NULL;
	//assert(_hhkShellHookProc == NULL);
}

/**
 * @brief 
*/
void HooksMgr::install(DWORD threadId /*= GetCurrentThreadId()*/)
{
	_hModule = load();

	//HOOKPROC hkShellHookProc = (HOOKPROC)GetProcAddress(_hModule, S(ShellHookProc));
	//assert(hkShellHookProc);

	HOOKPROC hkCallWndProc = (HOOKPROC)GetProcAddress(_hModule, S(CallWndProc));
	AssertTrue(hkCallWndProc, "Procedure adress should not be NULL");

	HOOKPROC hkGetMsgProc = (HOOKPROC)GetProcAddress(_hModule, S(GetMsgProc));
	AssertTrue(hkGetMsgProc, "Procedure adress should not be NULL");

	//_hhkShellHookProc = SetWindowsHookEx(WH_SHELL, hkShellHookProc, _hModule, NULL);
	//assert(_hhkShellHookProc);
	HookMapLock lock(_mutex);

	HHOOK _hhkCallWndProc = _callWndProcMap[threadId] = SetWindowsHookEx(WH_CALLWNDPROC, hkCallWndProc, _hModule, NULL);
	AssertTrue(_hhkCallWndProc, "Hook handle should not be NULL");

	HHOOK _hhkGetMessage = _getMessageMap[threadId] = SetWindowsHookEx(WH_GETMESSAGE, hkGetMsgProc, _hModule, NULL);
	AssertTrue(_hhkGetMessage, "Hook handle should not be NULL");
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

/**
 * @brief 
 * @return 
*/
bool HooksMgr::areHooksSet(DWORD threadId /*= GetCurrentThreadId()*/)
{
	HookMapLock lock(_mutex);
	return _callWndProcMap.find(threadId) != _callWndProcMap.end()
		|| _getMessageMap.find(threadId) != _getMessageMap.end();
}

/**
 * @brief starts other instance, if possible
*/
void HooksMgr::startOtherBitInstance()
{
	using std::regex;
	using std::regex_replace;
#ifdef _WIN64
	string exeName = regex_replace(Utils::ExeName, regex("64"), "32");
#else
	string exeName = regex_replace(Utils::ExeName, regex("32"), "64");
#endif // _WIN64
	char exePath[MAX_PATH] = { 0 };
	strcpy_s(exePath, Utils::BinDir.c_str());
	PathAppendA(exePath, exeName.c_str());
	DWORD fileAttributes = GetFileAttributesA(exePath);
	if (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		stopOtherBitInstance();

		STARTUPINFOA si = { sizeof(STARTUPINFO), 0 };
		char arguments[MAX_PATH] = "-no-tray";

		// Create the child process
		BOOL succeed = CreateProcessA(
			exePath,						 // Replace with the actual child process name or path
			arguments,					 // Command line (optional)
			NULL,							 // Process handle not inheritable
			NULL,							 // Thread handle not inheritable
			FALSE,						 // Set handle inheritance to FALSE
			0,								 // No creation flags
			NULL,							 // Use parent's environment block
			NULL,							 // Use parent's starting directory
			&si,							 // Pointer to STARTUPINFO structure
			&_childProcessInfo		 // Pointer to PROCESS_INFORMATION structure
		);
	}
}

void HooksMgr::stopOtherBitInstance()
{
	if (_childProcessInfo.hThread)
	{
		BOOL succeed = PostThreadMessageA(GetThreadId(_childProcessInfo.hThread), WM_QUIT, 0, 0);
		CloseHandle(_childProcessInfo.hProcess);
		CloseHandle(_childProcessInfo.hThread);
		_childProcessInfo = { 0 };
	}
}
