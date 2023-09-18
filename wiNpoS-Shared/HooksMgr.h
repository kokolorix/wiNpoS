#pragma once

#define MT_HOOK_MSG_CREATE_TASK_TOOLBAR RegisterWindowMessageA("wiNpoS-Hook.CreateTaskToolbar ")
#define MT_HOOK_MSG_DESTROY_TASK_TOOLBAR RegisterWindowMessageA("wiNpoS-Hook.DestroyTaskToolbar ")
#define MT_HOOK_MSG_UNLOAD RegisterWindowMessageA("wiNpoS-Hook.Unload")
#define MT_HOOK_MSG_REGISTER_THREAD_HOOK RegisterWindowMessageA("wiNpoS-Hook.RegisterThreadHook")

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
	HHOOK _hhkCallWndProc = NULL;
	HHOOK _hhkGetMessage = NULL;

public:
	static HMODULE load();
	static void unload(HMODULE hModule);

	void attach();
	void setHooks(HMODULE hModule);

	void detach();
	void unhookHooks();

	void install();
	void uninstall();
};

