#pragma once
#include <wtypes.h>
#include <map>
#include <vector>
#include "Utils.h"
#include "WinPosWndCfg.h"

/**
 * @brief 
*/
class WinPosWnd
{
public:
	void create(POINT pt, HWND hParentWnd);
	void destroy();

	FLOAT getScale() { return _scale; }
	HWND getWndHandle();

	static WinPosWnd* getWinPosWnd(HWND hWnd);

	void startCloseTimer();

private:
	using PosPreviewCfgs = WinPosWndCfg::PosPreviewCfgs;
	struct MonitorPreview;
	using MonitorPreviewPtr = std::shared_ptr<MonitorPreview>;

	HWND _hParentWnd = NULL;

	struct WinPosPreview
	{
		RECT wndRect;
		RECT prvRect;
		string name;
		std::weak_ptr<MonitorPreview> monitorPreview;
		void paint(HWND hWnd, PAINTSTRUCT& ps, HDC hDc) const;
		void onMouseMove(HWND hWnd, POINT pt);
		void onLButtonDown(HWND hWnd, POINT pt);
	};
	using WinPosPreviewPtr = std::shared_ptr<WinPosPreview>;
	using WinPosPreviews = std::vector<WinPosPreviewPtr>;

	struct MonitorPreview
	{
		WinPosWnd& winPosWnd;
		HWND hWnd;
		RECT monitorRect;
		RECT prvRect;
		string name;
		string device;
		WinPosPreviews previews;
		WinPosPreviewPtr activeWinPosPreview;
		void offsetToPt(POINT pt, RECT totalRect);
		void createWinPosPreviews(const PosPreviewCfgs& pcs, MonitorPreviewPtr mp);
		RECT getCrossRect();
		HWND create(HWND hParentWnd);
		WinPosPreviewPtr findPreview(POINT pt);
		void paint(PAINTSTRUCT& ps, HDC hDc);
		void onMouseMove(POINT pt);
		void onLButtonDown(POINT pt);
	};
	using MonitorPreviews = std::vector<std::shared_ptr<MonitorPreview>>;

private:
	static const WNDCLASS* wndClass;
	static std::map<HWND, WinPosWnd*> hWndToWinPosMap;
	std::vector<HWND>  _hWnds;

	MonitorPreviews _previews;

	RECT _totalRect = { 0 };
	UINT _closeTimeout = 3500;
	FLOAT _scale = 0.12f;

private:
	using RectVector = std::vector<RECT>;

	static WNDCLASS* initWndClass();
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	RECT getTotalPreviewRect(POINT pt, const RectVector& monitorRects);
	void correctEdgecases(POINT pt, const RECT& totalRect);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
