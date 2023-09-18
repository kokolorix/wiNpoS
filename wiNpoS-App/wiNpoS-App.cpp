// wiNpoS-App.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "wiNpoS-App.h"
#include "HooksMgr.h"
#include "Config.h"
#include "Utils.h"
#include <assert.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HooksMgr hooks;                                    // the hooks manager
Config config;                                  // the config manager

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI		  NewWindowProc(_In_ LPVOID lpParameter);
HWND                CreateNewWindow();

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

