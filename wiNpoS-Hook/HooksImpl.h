#pragma once

class HooksImpl
{
public:
	static LRESULT CALLBACK callWndProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);
	static LRESULT CALLBACK getMsgProc(_In_  int nCode, _In_  WPARAM wParam, _In_  LPARAM lParam);

private:
	static void onNcButtonDblClick(MSG* pMsg);
	static void onWindowPosChanged(CWPSTRUCT* pMsg);

};

