// wiNpoS-App.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "DebugNew.h"
#include "Utils.h"
#include "framework.h"
#include <windowsx.h>
#include "wiNpoS-App.h"
#include "HooksMgr.h"
#include "Config.h"
#include "Utils.h"
#include <Shlwapi.h>
#include <future>
#include <shellapi.h>
#include <regex>
#pragma comment(lib, "Shlwapi.lib")

#define MAX_LOADSTRING 100
#define MYWM_NOTIFYICON (WM_APP + 1)

// Global Variables:
HINSTANCE hInstance;                            // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
NOTIFYICONDATAA nid = { 0 };							// the Tray Icon struct

ConfigPtr config = std::make_unique<Config>();      ///> the config manager
HooksMgrPtr hooks = std::make_unique<HooksMgr>();   ///> the hooks manager

// Forward declarations of functions included in this code module:
ATOM						MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInst, int nCmdShow);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK		About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI			NewWindowProc(_In_ LPVOID lpParameter);
HWND						CreateNewWindow();
void						AttachToProcess(ULONG processId);
void						DetachFromProcess(ULONG targetPid, HINSTANCE hInstance);
void						DrawWindowBorderForTargeting(_In_ HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInst,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	 _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // TODO: Place code here.

    // Initialize global strings
#ifdef _WIN64
	 LoadStringW(hInst, IDS_APP_TITLE_64, szTitle, MAX_LOADSTRING);
#else
	 LoadStringW(hInst, IDS_APP_TITLE_32, szTitle, MAX_LOADSTRING);
#endif // _WIN64

    LoadStringW(hInst, IDC_WINPOSAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInst);

	 config->readConfig();

    // Perform application initialization:
    if (!InitInstance (hInst, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_WINPOSAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	 config->writeConfig();

	 Shell_NotifyIconA(NIM_DELETE, &nid);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINPOSAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINPOSAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINPOSAPP));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
   ::hInstance = hInst; // Store instance handle in our global variable

	using namespace std;

	string cmdLine = toLowerCase(string(GetCommandLineA()));
	bool installHook = cmdLine.find("not-install") == string::npos;
	bool showWnd = cmdLine.find("not-hidden") != string::npos;
	bool trayIcon = cmdLine.find("no-tray") == string::npos;
	bool noChildProcess = cmdLine.find("no-child") != string::npos;

	HWND hWnd = CreateNewWindow();
   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, showWnd ? nCmdShow : SW_HIDE);
   UpdateWindow(hWnd);

	HMENU hFileMenu = GetSubMenu(GetMenu(hWnd), 0);

	if(trayIcon)
	{
		nid = { sizeof(NOTIFYICONDATAA), hWnd, 0 };
		nid.uID = 1; // Unique ID for the notification icon
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = MYWM_NOTIFYICON;
		nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINPOSAPP)); // Set the icon
		if (lstrcpynA(nid.szTip, "wiNpoS, click to show more ...", sizeof(nid.szTip)))
			Shell_NotifyIconA(NIM_ADD, &nid);
		BOOL res = ModifyMenuA(hFileMenu, IDM_FILE_MINIMIZE_TO_TRAY, MF_STRING | MF_ENABLED | MF_BYCOMMAND, IDM_FILE_MINIMIZE_TO_TRAY, "&Minimize to tray");
	}

	if(installHook)
	{
		auto resFuture = std::async(std::launch::async, [hWnd]() {
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_FILE_INSTALL, 0), MAKELPARAM(0,0));
			});
	}

	if(trayIcon &&  !noChildProcess)
	{
		auto resFuture = std::async(std::launch::async, [hWnd]() {
			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_START_32_64_BIT, 0), MAKELPARAM(0, 0));
			});
	}

	int itemIndex = [hFileMenu](int itemCount)
		{
			for (int i = 0; i < itemCount; i++)
				if (GetMenuItemID(hFileMenu, i) == IDM_START_32_64_BIT)
					return  i;
			return -1;
		}(GetMenuItemCount(hFileMenu));

	if (itemIndex > -1)
	{
		using std::regex;
		using std::regex_replace;
#ifdef _WIN64
		string exeName = regex_replace(Utils::ExeName, regex("32"), "64");
#else
		string exeName = regex_replace(Utils::ExeName, regex("64"), "32");
#endif // _WIN64
		char exePath[MAX_PATH] = { 0 };
		strcpy_s(exePath, Utils::BinDir.c_str());
		PathAppendA(exePath, exeName.c_str());
		DWORD fileAttributes = GetFileAttributesA(exePath);

		UINT nFlags = (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			? MF_STRING | MF_ENABLED
			: MF_STRING | MF_DISABLED | MF_GRAYED
			;

#ifdef _WIN64
		BOOL res = ModifyMenuA(hFileMenu, IDM_START_32_64_BIT, nFlags|MF_BYCOMMAND, IDM_START_32_64_BIT, "&Start 32 bit");
#else
		BOOL res = ModifyMenuA(hFileMenu, itemIndex, nFlags|MF_BYPOSITION, IDM_START_32_64_BIT, "&Start 64 bit");
#endif // _WIN64
		DrawMenuBar(hWnd);
	}

   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
namespace
{
	DWORD targetingWindowId = 0;
	//bool targetingWindow = false;
	HWND hTargetingCurrentWnd = NULL;
}
/**
* @def IDM_FILE_ATTACH
* @brief @ref IDM_FILE_ATTACH "Attach to Window"
* @msc
* 
* WndProc[label="WndProc(...)", URL="@ref WndProc"],
* GetMsgProc[label="GetMsgProc(...)", URL="@ref GetMsgProc"],
* Menu;
* 
* WndProc->GetMsgProc[label="PostMessage", URL="@ref MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK"];
* ...;
* WndProc=>>Menu[label="CheckMenuItem"];
* WndProc=>>Menu[label="EnableMenuItem"];
* 
* @endmsc
*/

/**
 * @brief the main window procedure of this app
 * @param hWnd 
 * @param message 
 * @param wParam 
 * @param lParam 
 * 
 * @copydoc IDM_FILE_ATTACH
 * 
 * @return 
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
				case IDM_TRAYMENU_EXIT:
					PostMessage(HWND_BROADCAST, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
					Sleep(1000);
					hooks->stopOtherBitInstance();
					hooks->uninstall();
					DestroyWindow(hWnd);
					break;
				case IDM_FILE_ATTACH:
					hooks->loadHook();
					PostMessage(hWnd, MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_ATTACH, MF_CHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_ATTACH, MF_DISABLED);
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_DETACH, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_DETACH, MF_ENABLED);
					DrawMenuBar(hWnd);
					break;
				case IDM_FILE_DETACH:
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_ATTACH, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_ATTACH, MF_ENABLED);
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_DETACH, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_DETACH, MF_DISABLED);
					DrawMenuBar(hWnd);
					PostMessage(hWnd, MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
					PostMessage(hWnd, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
					PostMessage(hWnd, MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK, (WPARAM)hWnd, (LPARAM)GetCurrentThreadId());
					//hooks->detach();
					break;
				case IDM_FILE_INSTALL:
					hooks->install();
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_CHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_DISABLED);
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_ENABLED);
					DrawMenuBar(hWnd);
					break;
				case IDM_FILE_UNINSTALL:
					PostMessage(HWND_BROADCAST, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, (WPARAM)0, (LPARAM)0);
					hooks->uninstall();
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_ENABLED);
					CheckMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_UNCHECKED);
					EnableMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_DISABLED);
					DrawMenuBar(hWnd);
					break;
				case IDM_FILE_OPEN_NEW_WND:
					CreateThread(NULL, 0, NewWindowProc, NULL, 0, NULL);
					break;
				case IDM_FILE_ATTACH_TO_WND:
					SetCapture(hWnd);
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					targetingWindowId = wmId;
					hTargetingCurrentWnd = NULL;
					break;
				case IDM_FILE_DETACH_FROM_WND:
					SetCapture(hWnd);
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					targetingWindowId = wmId;
					hTargetingCurrentWnd = NULL;
					break;
				case IDM_FILE_SEND_UNLOAD:
					WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_UNLOAD) to all Windows", MT_HOOK_MSG_UNLOAD));
					PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
					break;
				case IDM_FILE_SEND_CREATE_TASK_TOOLBAR:
					WRITE_DEBUG_LOG(format("Send message {}(IDM_FILE_SEND_CREATE_TASK_TOOLBAR) to all Windows", MT_HOOK_MSG_CREATE_TASK_TOOLBAR));
					PostMessage(HWND_BROADCAST, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, 0, 0);
					break;
				case IDM_FILE_SEND_DESTROY_TASK_TOOLBAR:
					WRITE_DEBUG_LOG(format("Send message {}(IDM_FILE_SEND_DESTROY_TASK_TOOLBAR) to all Windows", MT_HOOK_MSG_DESTROY_TASK_TOOLBAR));
					PostMessage(HWND_BROADCAST, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, 0, 0);
					break;
				case IDM_FILE_OPEN_CONFIG_DIR:
					config->openFolder();
					break;
				case IDM_FILE_OPEN_WINPOS_CONFIG:
					config->openWinPosConfig();
					break;
				case IDM_START_32_64_BIT:
					hooks->startOtherBitInstance();
					break;
				case IDM_FILE_MINIMIZE_TO_TRAY:
					ShowWindow(hWnd, SW_HIDE);
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

		case WM_LBUTTONDOWN:
		{
			if (targetingWindowId)
			{
				// Send the window to the bottom.
				SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
				SendMessage(hWnd, WM_MOUSEMOVE, 0, 0);
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (targetingWindowId)
			{
				POINT cursorPos;
				HWND hWndUnderMouse;

				GetCursorPos(&cursorPos);
				hWndUnderMouse = WindowFromPoint(cursorPos);
				ULONG processId = 0;
				ULONG threadId = GetWindowThreadProcessId(hWndUnderMouse, &processId);
				hWndUnderMouse = GetMainWnd(threadId, processId);
				if (hWndUnderMouse != hWnd)
				{
					//FlashWindow(windowUnderMouse, true);
					DrawWindowBorderForTargeting(hWndUnderMouse);
					hTargetingCurrentWnd = hWndUnderMouse;
				}
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			if (targetingWindowId && hTargetingCurrentWnd != hWnd)
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));

				// Bring the window back to the top, and preserve the Always on Top setting.
				SetWindowPos(hTargetingCurrentWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

				ULONG processId = 0;
				ULONG threadId = GetWindowThreadProcessId(hTargetingCurrentWnd, &processId);

				RECT rect;
				GetWindowRect(hWnd, &rect);
				InvalidateRect(hWnd, &rect, TRUE);

				switch (targetingWindowId)
				{
					case IDM_FILE_ATTACH_TO_WND:
						AttachToProcess(processId);
						WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK) to {:#018x}", MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK, (uint64_t)hTargetingCurrentWnd));
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
						break;
					case IDM_FILE_DETACH_FROM_WND:
						WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK) to {:#018x}", MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK, (uint64_t)hTargetingCurrentWnd));
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_UNREGISTER_WND_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_UNREGISTER_SUPPORT_THREAD_HOOK, (WPARAM)hWnd, (LPARAM)GetCurrentThreadId());
						break;
					case IDM_FILE_SEND_CREATE_TASK_TOOLBAR:
						WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_CREATE_TASK_TOOLBAR) to {:#018x}", MT_HOOK_MSG_CREATE_TASK_TOOLBAR, (uint64_t)hTargetingCurrentWnd));
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_CREATE_TASK_TOOLBAR, 0, 0);
						break;
					case IDM_FILE_SEND_DESTROY_TASK_TOOLBAR:
						WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_DESTROY_TASK_TOOLBAR) to {:#018x}", MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, (uint64_t)hTargetingCurrentWnd));
						PostMessage(hTargetingCurrentWnd, MT_HOOK_MSG_DESTROY_TASK_TOOLBAR, 0, 0);
						break;
					default:
						break;
				}

				targetingWindowId = 0;
				hTargetingCurrentWnd = NULL;
				ReleaseCapture();

				//AttachToProcess(processId);
			}
		}
		break;

		case WM_KEYDOWN:
		{
			if (targetingWindowId)
			{
				if (wParam == VK_ESCAPE)
				{
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					ReleaseCapture();
					targetingWindowId = 0;
					hTargetingCurrentWnd = NULL;
				}
			}
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;

		case WM_DESTROY:
			GetWindowRect(hWnd, &config->Rect);
			SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(0, IDM_FILE_UNINSTALL), 0);
			PostQuitMessage(0);
			break;

		case MYWM_NOTIFYICON:
			switch (lParam) {
				case WM_RBUTTONDOWN:
				case WM_CONTEXTMENU:
					// Show context menu
				{
					POINT cursor;
					GetCursorPos(&cursor);
					HMENU hMenu = LoadMenu(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDR_TRAY_MENU));
					TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
				}
				break;
				case WM_LBUTTONUP:
					// Handle left-click event
				{
					if (IsWindowVisible(hWnd))
					{
						ShowWindow(hWnd, SW_HIDE);
					}
					else
					{
						ShowWindow(hWnd, SW_SHOW);
						SetForegroundWindow(hWnd); // Ensure that the window is in the foreground
					}
				}
				break;
			}
			break;

		default:
			if (message == MT_HOOK_MSG_SUPPORT_THREAD_HOOK_UNREGISTERED)
			{
				HWND hSrcWnd = (HWND)wParam;
				if (hWnd == hSrcWnd)
				{
					hooks->unloadHook();
				}
				else
				{
					DWORD wndProcessId = NULL;
					auto wndThreadId = GetWindowThreadProcessId(hSrcWnd, &wndProcessId);
					HINSTANCE hInstance = (HINSTANCE)lParam;
					DetachFromProcess(wndProcessId, hInstance);
				}
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	 {
		 //HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		 //if(hResource == NULL)
			// return (INT_PTR)FALSE;

		 //HGLOBAL hResourceData = LoadResource(NULL, hResource);
		 //if(hResourceData == NULL)
			// return (INT_PTR)FALSE;

		 //VS_FIXEDFILEINFO* vi = (VS_FIXEDFILEINFO*)LockResource(hResourceData);
		 //if(vi == NULL)
			// return (INT_PTR)FALSE;

		 HWND hStatic = GetDlgItem(hDlg, IDC_STATIC_VERSION); // Get the handle of the static control
		 if (hStatic == NULL) 
			 return (INT_PTR)FALSE;

		 string text = format("{}, Version {}", Utils::ProductName, Utils::ProductVersion);
		 SetWindowTextA(hStatic, text.c_str()); // Set the text for the static control

		 return (INT_PTR)TRUE;
	 }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

DWORD WINAPI NewWindowProc(_In_ LPVOID lpParameter)
{
	HWND hNewOne = CreateNewWindow();
	PostMessage(hNewOne, MT_HOOK_MSG_REGISTER_WND_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId());
	ShowWindow(hNewOne, SW_SHOW);
	UpdateWindow(hNewOne);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

HWND CreateNewWindow()
{
	std::wstring title = dwformat(L"{} - {}({})", szTitle, GetCurrentProcessId(), GetCurrentThreadId());
	HWND hWnd = CreateWindow(
		szWindowClass,
		title.c_str(),
		WS_OVERLAPPEDWINDOW,
		config->Rect.left,
		config->Rect.top,
		config->Rect.right - config->Rect.left,
		config->Rect.bottom - config->Rect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return hWnd;
}

void AttachToProcess(ULONG targetPid)
{
	// Open the target process with read/write access
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
	if (hProcess == NULL)
		return Utils::ShowLastError("Failed to open process, error {}");

	// Get the address of the LoadLibrary function in the kernel32.dll module
	HMODULE hKernel32 = GetModuleHandle(_T("kernel32.dll"));
	if (hKernel32 == NULL)
		return Utils::ShowLastError("Failed to obtain kernel32.dll handle, error {}");

	FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
	if (pLoadLibrary == nullptr)
		return Utils::ShowLastError("Failed to obtain LoadLibraryW adress, error {}");

	WCHAR path[MAX_PATH];
	DWORD pathLength = GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
#ifdef _WIN64
	PathAppend(path, L"wiNpoS-Support64.dll");
#else
	PathAppend(path, L"wiNpoS-Support32.dll");
#endif // _WIN64

	// Allocate memory in the target process to hold the path to the DLL
	SIZE_T pathSize = (_tcslen(path) * sizeof(_TCHAR));
	LPVOID remotePath = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
	if (remotePath == NULL)
	{
		Utils::ShowLastError("Failed to allocate memory in process, error {}");
		CloseHandle(hProcess);
		return;
	}

	// Write the path to the DLL to the allocated memory in the target process
	if (!WriteProcessMemory(hProcess, remotePath, path, pathSize, NULL))
	{
		Utils::ShowLastError("Failed to write to process memory, error {}");
		VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return;
	}

	// Create a remote thread in the target process to call LoadLibrary with the path to the DLL
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, remotePath, 0, NULL);
	if (hThread == NULL)
	{
		Utils::ShowLastError("Failed to create remote thread, error {}");
		VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return;
	}

	// Wait for the remote thread to finish executing
	WaitForSingleObject(hThread, INFINITE);

	// Free the memory and close the handles
	VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
}

void DetachFromProcess(ULONG targetPid, HINSTANCE hInstance)
{
	// Open the target process with read/write access
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
	if (hProcess == NULL)
		return Utils::ShowLastError("Failed to open process, error {}");

	// Get the address of the LoadLibrary function in the kernel32.dll module
	HMODULE hKernel32 = GetModuleHandle(_T("kernel32.dll"));
	if (hKernel32 == NULL)
		return Utils::ShowLastError("Failed to obtain kernel32.dll handle, error {}");

	FARPROC pFreeLibrary = GetProcAddress(hKernel32, "FreeLibrary");
	if (pFreeLibrary == nullptr)
		return Utils::ShowLastError("Failed to obtain LoadLibraryW adress, error {}");


	// Allocate memory in the target process to hold the path to the DLL
	//SIZE_T hSize = sizeof(HMODULE);
	//LPVOID hRemote = VirtualAllocEx(hProcess, NULL, hSize, MEM_COMMIT, PAGE_READWRITE);
	//if (hRemote == NULL)
	//{
	//	Utils::ShowLastError("Failed to allocate memory in process, error {}");
	//	CloseHandle(hProcess);
	//	return;
	//}

	//// Write the path to the DLL to the allocated memory in the target process
	//if (!WriteProcessMemory(hProcess, hRemote, hInstance, hSize, NULL))
	//{
	//	Utils::ShowLastError("Failed to write to process memory, error {}");
	//	VirtualFreeEx(hProcess, hRemote, 0, MEM_RELEASE);
	//	CloseHandle(hProcess);
	//	return;
	//}

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	// Create a remote thread in the target process to call LoadLibrary with the path to the DLL
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, si.dwPageSize * 32, (LPTHREAD_START_ROUTINE)pFreeLibrary, (LPVOID)hInstance, 0, NULL);
	if (hThread == NULL)
	{
		Utils::ShowLastError("Failed to create remote thread, error {}");
		//VirtualFreeEx(hProcess, hRemote, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return;
	}

	// Wait for the remote thread to finish executing
	WaitForSingleObject(hThread, INFINITE);

	// Free the memory and close the handles
	//VirtualFreeEx(hProcess, hRemote, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
}

void DrawWindowBorderForTargeting(_In_ HWND hWnd)
{
	RECT rect;
	HDC hdc;

	GetWindowRect(hWnd, &rect);
	hdc = GetWindowDC(hWnd);

	if (hdc)
	{
		INT penWidth;
		INT oldDc;
		HPEN pen;
		HBRUSH brush;

		penWidth = GetSystemMetrics(SM_CXBORDER) * 10;

		oldDc = SaveDC(hdc);

		// Get an inversion effect.
		SetROP2(hdc, R2_NOT);

		pen = CreatePen(PS_INSIDEFRAME, penWidth, RGB(0x00, 0x00, 0x00));
		SelectPen(hdc, pen);

		brush = GetStockBrush(NULL_BRUSH);
		SelectBrush(hdc, brush);

		// Draw the rectangle.
		Rectangle(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

		// Cleanup.
		DeletePen(pen);

		RestoreDC(hdc, oldDc);
		ReleaseDC(hWnd, hdc);
	}
}
