#include "pch.h"
#include "HooksMgr.h"
#include <cassert>
#include <regex>
#include "Utils.h"
#include <Shlwapi.h>

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
/**
 * @brief starts other instance, if possible
*/
namespace
{
	void atExit()
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
		std::system(format("taskkill /F /IM {}", exePath).c_str());

	}
}
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
			arguments,						 // Command line (optional)
			NULL,							 // Process handle not inheritable
			NULL,							 // Thread handle not inheritable
			FALSE,							 // Set handle inheritance to FALSE
			0,								 // No creation flags
			NULL,							 // Use parent's environment block
			NULL,							 // Use parent's starting directory
			&si,							 // Pointer to STARTUPINFO structure
			&_childProcessInfo				 // Pointer to PROCESS_INFORMATION structure
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
