#pragma once
struct ITaskbarList3;

class TaskToolbar
{
public:
	TaskToolbar();
	~TaskToolbar();

	HRESULT initialize(HINSTANCE hInst, HWND hWnd);
	void uninitialize(HINSTANCE hInst, HWND hWnd);

private:
	ITaskbarList3* _pTaskbarList = nullptr;
};

