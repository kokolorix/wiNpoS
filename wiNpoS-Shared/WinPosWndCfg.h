#pragma once
#include <wtypes.h>
#include <map>
#include <vector>
#include "Utils.h"

/**
 * @brief 
*/
class WinPosWndCfg
{
public:
	struct PosPreviewConfig
	{
		RECT wndRect;
		RECT prvRect;
		string name;
		Units units;
	};
	using PosPreviewCfgs = std::vector<PosPreviewConfig>;

	struct MonitorConfig
	{
		RECT monitorRect;
		RECT previewRect;
		string device;
		string name;
		uint32_t index;
		PosPreviewCfgs previews;
	};
	using Monitors = std::vector<MonitorConfig>;

public:
	void readConfig();
	void writeConfig();

	WinPosWndCfg::Monitors getMonitors() const { return _monitors; }
	UINT getCloseTimeout() const { return _closeTimeout; }
	FLOAT getScale() const { return _scale; }

	static string getConfigPath(const string& configName = "WndConfig");

private:
	UINT _closeTimeout = 3500;
	FLOAT _scale = 0.12f;

	Monitors _monitors;
};

