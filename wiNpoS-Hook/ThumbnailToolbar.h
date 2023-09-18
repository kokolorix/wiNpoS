#pragma once
class ThumbnailToolbar
{
public:
	ThumbnailToolbar();
	~ThumbnailToolbar();

	HRESULT initialize(HINSTANCE hInst, HWND hWnd);
	void uninitialize();
};

