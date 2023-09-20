// wiNpoS-App.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include <windowsx.h>
#include "wiNpoS-App.h"
#include "Utils.h"
#include "HooksMgr.h"
#include "Config.h"
#include "Utils.h"
#include <assert.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HooksMgr hooks;                                    // the hooks manager
Config config;                                  // the config manager

// Forward declarations of functions included in this code module:
ATOM						MyRegisterClass(HINSTANCE hInstance);
BOOL						InitInstance(HINSTANCE, int);
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK		About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI			NewWindowProc(_In_ LPVOID lpParameter);
HWND						CreateNewWindow();
void						AttachToProcess(ULONG processId);
void						DrawWindowBorderForTargeting(_In_ HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
#ifdef _WIN64
	 LoadStringW(hInstance, IDS_APP_TITLE_64, szTitle, MAX_LOADSTRING);
#else
	 LoadStringW(hInstance, IDS_APP_TITLE_32, szTitle, MAX_LOADSTRING);
#endif // _WIN64

    LoadStringW(hInstance, IDC_WINPOSAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	 config.readConfig();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINPOSAPP));

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

	 config.writeConfig();

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
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateNewWindow();
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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

	bool targetingWindow = false;
	HWND targetingCurrentWindow = NULL;
}
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
					 DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					 break;
				 case IDM_EXIT:
					 DestroyWindow(hWnd);
					 break;
				 case IDM_FILE_ATTACH:
					 hooks.attach();
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
					 hooks.detach();
					 break;
				 case IDM_FILE_INSTALL:
					 hooks.install();
					 CheckMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_CHECKED);
					 EnableMenuItem(GetMenu(hWnd), IDM_FILE_INSTALL, MF_DISABLED);
					 CheckMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_UNCHECKED);
					 EnableMenuItem(GetMenu(hWnd), IDM_FILE_UNINSTALL, MF_ENABLED);
					 DrawMenuBar(hWnd); 
					 break;
				 case IDM_FILE_UNINSTALL:
					 hooks.uninstall();
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
					 {
						 SetCapture(hWnd);
						 SetCursor(LoadCursor(NULL, IDC_CROSS));
						 targetingWindow = true;
						 targetingCurrentWindow = NULL;
					 }
					 break;
				 case IDM_FILE_SEND_UNLOAD:
					 WRITE_DEBUG_LOG(format("Send message {}(MT_HOOK_MSG_UNLOAD) to all Windows", MT_HOOK_MSG_UNLOAD));
					 assert(PostMessage(HWND_BROADCAST, MT_HOOK_MSG_UNLOAD, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId()));
					 break;
				 case IDM_FILE_OPEN_CINFIG_DIR:
                config.openFolder();
					 break;

				 default:
					 return DefWindowProc(hWnd, message, wParam, lParam);
			 }
		 }
		 break;
		 case WM_LBUTTONDOWN:
			 {
				 if (targetingWindow)
				 {
					 // Send the window to the bottom.
					 SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
					 SendMessage(hWnd, WM_MOUSEMOVE, 0, 0);
				 }
			 }
			 break;
		 case WM_MOUSEMOVE:
			 {
				 if (targetingWindow)
				 {
					 POINT cursorPos;
					 HWND windowUnderMouse;

					 GetCursorPos(&cursorPos);
					 windowUnderMouse = WindowFromPoint(cursorPos);
					 ULONG processId = 0;
					 ULONG threadId = GetWindowThreadProcessId(targetingCurrentWindow, &processId);
					 threadId = threadId;
					 if(windowUnderMouse != hWnd)
					 {
						 //FlashWindow(windowUnderMouse, true);
						 DrawWindowBorderForTargeting(windowUnderMouse);
						 targetingCurrentWindow = windowUnderMouse;
					 }
				 }
			 }
			 break;
		 case WM_LBUTTONUP:
			 {
				 if (targetingWindow && targetingCurrentWindow != hWnd)
				 {
					 SetCursor(LoadCursor(NULL, IDC_ARROW));

					 // Bring the window back to the top, and preserve the Always on Top setting.
					 SetWindowPos(targetingCurrentWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

					 ULONG processId = 0;
					 ULONG threadId = GetWindowThreadProcessId(targetingCurrentWindow, &processId);

					 RECT rect;
					 GetWindowRect(hWnd, &rect);
					 InvalidateRect(hWnd, &rect, TRUE);

					 targetingWindow = false;
					 targetingCurrentWindow = NULL;
					 ReleaseCapture();

					 AttachToProcess(processId);
				 }
			 }
			 break;
		 case WM_KEYDOWN :
			 {
				 if (targetingWindow)
				 {
					 if (wParam == VK_ESCAPE)
					 {
						 SetCursor(LoadCursor(NULL, IDC_ARROW));
						 ReleaseCapture();
						 targetingWindow = false;
						 targetingCurrentWindow = NULL;
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
			 GetWindowRect(hWnd, &config.Rect);
			 PostQuitMessage(0);
			 break;
		 default:
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
        return (INT_PTR)TRUE;

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
	assert(PostMessage(hNewOne, MT_HOOK_MSG_REGISTER_THREAD_HOOK, (WPARAM)GetCurrentProcessId(), (LPARAM)GetCurrentThreadId()));
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
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		config.Rect.left,
		config.Rect.top,
		config.Rect.right - config.Rect.left,
		config.Rect.bottom - config.Rect.top,
		nullptr,
		nullptr,
		hInst,
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
	PathAppend(path, L"wiNpoS-Hook64.dll");
#else
	PathAppend(path, L"wiNpoS-Hook32.dll");
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

		penWidth = GetSystemMetrics(SM_CXBORDER) * 3;

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
