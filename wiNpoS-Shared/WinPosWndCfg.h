#pragma once
#include <wtypes.h>
#include <map>
#include <vector>
#include "Utils.h"
#include <chrono>
#include <atomic>

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
	UINT getOpenTimeout() const { return _openTimeout; }
	FLOAT getScale() const { return _scale; }

	static string getConfigPath(const string& configName = "WndConfig");

private:
	UINT _closeTimeout = 800;
	UINT _openTimeout = 300;
	FLOAT _scale = 0.12f;

	Monitors _monitors;
	static std::atomic<std::chrono::time_point<std::chrono::file_clock>> _lastWriteTimeStamp;
	static std::shared_ptr<WinPosWndCfg> _configCache;
};

