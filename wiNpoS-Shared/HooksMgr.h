#pragma once
#include "Utils.h"

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
	HHOOK _hhkShellHookProc = NULL;
	HHOOK _hhkCallWndProc = NULL;
	HHOOK _hhkGetMessage = NULL;
	PROCESS_INFORMATION _childProcessInfo = { 0 };

public:
	static HMODULE load();
	static void unloadHook(HMODULE hModule);
	void loadHook();
	void unloadHook();

	void attach();
	void setHooks(HMODULE hModule);

	void detach();
	void unhookHooks();

	void install();
	void uninstall();

	void startOtherBitInstance();
	void stopOtherBitInstance();
};



using HooksMgrPtr = std::unique_ptr<HooksMgr>;
#ifdef _DOXYGEN_RUNNING
namespace std { template<> class unique_ptr<HooksMgr> { HooksMgr* p; operator HooksMgr* () { return p; } HooksMgr* operator -> () { return p; } }; }
#endif
