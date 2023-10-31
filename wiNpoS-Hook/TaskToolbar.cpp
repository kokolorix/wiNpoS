#include "pch.h"
#include "DebugNew.h"
#include "TaskToolbar.h"
#include "resource.h"

#include <shobjidl_core.h>   // For ITaskbarList3
#include <strsafe.h>
#include <assert.h>
#include "Utils.h"
#pragma comment(lib, "comctl32.lib")

TaskToolbar::TaskToolbar()
{

}

TaskToolbar::~TaskToolbar()
{

}

HRESULT TaskToolbar::initialize(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_pTaskbarList));
	WRITE_DEBUG_LOG(dformat("hWnd: {:#018x}, pTaskbarList: {:#018x}", (uint64_t)hWnd, (uint64_t)_pTaskbarList));
	if (SUCCEEDED(hr))
	{
		hr = _pTaskbarList->HrInit();
		if (SUCCEEDED(hr))
		{
			//typedef struct THUMBBUTTON
			//{
			//	THUMBBUTTONMASK dwMask;
			//	UINT iId;
			//	UINT iBitmap;
			//	HICON hIcon;
			//	WCHAR szTip[260];
			//	THUMBBUTTONFLAGS dwFlags;
			//} 	THUMBBUTTON;

			static const THUMBBUTTONMASK mask = THB_ICON | THB_TOOLTIP | THB_FLAGS;
			static const THUMBBUTTONFLAGS flags = THBF_ENABLED;
			THUMBBUTTON buttons[] = 
			{
				 {mask, ID_SYSMENU_GROW,		0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_GROW)),		L"Grow the window",					flags }
				,{mask, ID_SYSMENU_SHRINK,		0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_SHRINK)),		L"Shrink the window",				flags }
				,{mask, ID_SYSMENU_GROW_H,		0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_GROW_H)),		L"Grow the window horizontally", flags }
				,{mask, ID_SYSMENU_SHRINK_H,	0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_SHRINK_H)),	L"Shrink the window horizontally",flags }
				,{mask, ID_SYSMENU_GROW_V,		0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_GROW_V)),		L"Grow the window vertically",	flags }
				,{mask, ID_SYSMENU_SHRINK_V,	0, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_SHRINK_V)),	L"Shrink the window vertically",	flags }
				,{mask, ID_SYSMENU_SHOWPOSWINDOW,	0,
								LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_WIN_POS)),	L"Shows the positioning window",		flags | THBF_DISMISSONCLICK }
			};

			// Set the buttons to be the thumbnail toolbar
			hr = _pTaskbarList->ThumbBarAddButtons(hWnd, ARRAYSIZE(buttons), buttons);
			
		}

		// It's OK to release ITaskbarList3 here; the thumbnail toolbar will remain.
		_pTaskbarList->Release();
		_pTaskbarList = nullptr;
	}
	return hr;
	//assert(SUCCEEDED(hr));o
}

void TaskToolbar::uninitialize(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_pTaskbarList));
	WRITE_DEBUG_LOG(dformat("hWnd: {:#018x}, pTaskbarList: {:#018x}", (uint64_t)hWnd, (uint64_t)_pTaskbarList));
	if (SUCCEEDED(hr))
	{
		hr = _pTaskbarList->HrInit();
		if (SUCCEEDED(hr))
		{
			_pTaskbarList->DeleteTab(hWnd);
			_pTaskbarList->AddTab(hWnd);
			//struct THUMBBUTTON
			//{
			//	THUMBBUTTONMASK dwMask;
			//	UINT iId;
			//	UINT iBitmap;
			//	HICON hIcon;
			//	WCHAR szTip[260];
			//	THUMBBUTTONFLAGS dwFlags;
			//}

			//THUMBBUTTON buttons[3] = {
			//	 {THB_FLAGS, ID_SYSMENU_INCREMENTWINDOWSIZE, 0, 0, 0, THBF_HIDDEN }
			//	,{THB_FLAGS, ID_SYSMENU_DECREMENTWINDOWSIZE, 0, 0, 0, THBF_HIDDEN }
			//	,{THB_FLAGS, ID_SYSMENU_SHOWPOSWINDOW,			0, 0, 0, THBF_HIDDEN }
			//};
			//hr = _pTaskbarList->ThumbBarUpdateButtons(hWnd, ARRAYSIZE(buttons), buttons);
			
		}

		// It's OK to release ITaskbarList3 here; the thumbnail toolbar will remain.
		_pTaskbarList->Release();
		_pTaskbarList = nullptr;
	}
}
