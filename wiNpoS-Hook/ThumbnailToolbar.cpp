#include "pch.h"
#include "ThumbnailToolbar.h"
#include "resource.h"

#include <shobjidl_core.h>   // For ITaskbarList3
#include <strsafe.h>
#include <assert.h>
#pragma comment(lib, "comctl32.lib")

ThumbnailToolbar::ThumbnailToolbar()
{

}

ThumbnailToolbar::~ThumbnailToolbar()
{

}

HRESULT ThumbnailToolbar::initialize(HINSTANCE hInst, HWND hWnd)
{
	ITaskbarList3* pTaskbarList;
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskbarList));
	if (SUCCEEDED(hr))
	{
		hr = pTaskbarList->HrInit();
		if (SUCCEEDED(hr))
		{
			HICON hWin		= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_WIN));
			HICON hWinInc	= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_WIN_INC));
			HICON hWinDec	= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_WIN_DEC));

			if (hWin && hWinInc && hWinDec)
			{
				THUMBBUTTON buttons[3] = {};

				// First button
				buttons[0].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS;
				buttons[0].dwFlags = THBF_ENABLED;
				buttons[0].iId = ID_SYSMENU_INCREMENTWINDOWSIZE;
				buttons[0].hIcon = hWinInc;
				StringCchCopy(buttons[0].szTip, ARRAYSIZE(buttons[0].szTip), L"Increment Window Size");

				// Second button
				buttons[1].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS;
				buttons[1].dwFlags = THBF_ENABLED;
				buttons[1].iId = ID_SYSMENU_DECREMENTWINDOWSIZE;
				buttons[1].hIcon = hWinDec;
				StringCchCopy(buttons[1].szTip, ARRAYSIZE(buttons[1].szTip), L"Decrement Window Size");

				// Third button
				buttons[2].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS;
				buttons[2].dwFlags = THBF_ENABLED | THBF_DISMISSONCLICK;
				buttons[2].iId = ID_SYSMENU_SHOWPOSWINDOW;
				buttons[2].hIcon = hWin;
				StringCchCopy(buttons[2].szTip, ARRAYSIZE(buttons[2].szTip), L"Open Positioning Window");

				// Set the buttons to be the thumbnail toolbar
				hr = pTaskbarList->ThumbBarAddButtons(hWnd, ARRAYSIZE(buttons), buttons);
			}
		}

		// It's OK to release ITaskbarList3 here; the thumbnail toolbar will remain.
		pTaskbarList->Release();
	}
	return hr;
	//assert(SUCCEEDED(hr));
}

void ThumbnailToolbar::uninitialize()
{

}
