#pragma once
#include "Utils.h"
#include <map>

#define MT_HOOK_MSG_CREATE_TASK_TOOLBAR RegisterWindowMessageA("wiNpoS-Hook.CreateTaskToolbar")
#define MT_HOOK_MSG_DESTROY_TASK_TOOLBAR RegisterWindowMessageA("wiNpoS-Hook.DestroyTaskToolbar")
#define MT_HOOK_MSG_UNLOAD RegisterWindowMessageA("wiNpoS-Hook.Unload")
#define MT_HOOK_MSG_REGISTER_SUPPORT_THREAD_HOOK RegisterWindowMessageA("wiNpoS-Hook.RegisterSupportThreadHook")
#define MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK RegisterWindowMessageA("wiNpoS-Hook.UnregisterSupportThreadHook")
#define MT_HOOK_MSG_SUPPORT_THREAD_HOOK_UNREGISTERED RegisterWindowMessageA("wiNpoS-Hook.SupportThreadHookUnregistered")
#define MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK RegisterWindowMessageA("wiNpoS-Hook.RegisterWndThreadHook")
#define MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK RegisterWindowMessageA("wiNpoS-Hook.UnregisterWndThreadHook")

/**
 * @brief Manage the Hooks install / uninstall
*/
class HooksMgr
{
public:
	HooksMgr() = default;
	~HooksMgr() = default;

private:
	HMODULE _hModule = NULL;
	//HHOOK _hhkShellHookProc = NULL;
	using HookMap = std::map<DWORD, HHOOK>;
	using HookMapMutex = std::mutex;
	using HookMapLock = std::lock_guard<HookMapMutex>;
	HookMapMutex _mutex;
	HookMap _callWndProcMap;
	HookMap _getMessageMap;
	//HHOOK _hhkCallWndProc = NULL;
	//HHOOK _hhkGetMessage = NULL;
	PROCESS_INFORMATION _childProcessInfo = { 0 };

public:
	static HMODULE load();
	static void unloadHook(HMODULE hModule);

	void loadHook();
	void unloadHook();

	void attach();
	void setHooks(DWORD threadId = GetCurrentThreadId());
	void setHooks(HMODULE hModule, DWORD threadId = GetCurrentThreadId());

	void detach();
	void unhookHooks(DWORD threadId = GetCurrentThreadId());

	void install(DWORD threadId = GetCurrentThreadId());
	void uninstall();

	bool areHooksSet(DWORD threadId = GetCurrentThreadId());

	void startOtherBitInstance();
	void stopOtherBitInstance();
};



using HooksMgrPtr = std::unique_ptr<HooksMgr>;
#ifdef _DOXYGEN_RUNNING
namespace std { template<> class unique_ptr<HooksMgr> { HooksMgr* p; operator HooksMgr* () { return p; } HooksMgr* operator -> () { return p; } }; }
#endif
