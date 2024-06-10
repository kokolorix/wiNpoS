#pragma once
#include <wtypes.h>
#include <string>
#include "Utils.h"
#include <chrono>
#include <atomic>

/**
 * @brief real complicated forward declaration
*/
namespace rapidjson
{
	template<typename T>
	struct UTF8;

	template<typename T>
	class MemoryPoolAllocator;
	class CrtAllocator;

	template <typename E, typename A, typename SA>
	class GenericDocument;
	typedef GenericDocument<UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator> Document;
}
using DocumentPtr = std::shared_ptr<rapidjson::Document>;

class HooksCfg
{
public:
	RECT Rect = { 0 };
	bool _disabled = false;

public:
	void readAppConfig();
	void readConfig();

	void writeAppConfig();
	void writeConfig();

	void openFolder();
	void openWinPosConfig();

	void disable();
	void enable();
	bool isDisabled();

private:
	DocumentPtr readConfigDocument(const string& configPath);
	void writeConfigDocument(rapidjson::Document& d, const string& configPath);
	string getAppConfigPath();
	string getConfigPath();
	string getConfigDir();

	static std::atomic<std::chrono::time_point<std::chrono::file_clock>> _lastWriteTimeStamp;
	static std::shared_ptr<HooksCfg> _configCache;
};

using HooksCfgPtr = std::unique_ptr<HooksCfg>;
#ifdef _DOXYGEN_RUNNING
namespace std { template<> class unique_ptr<HooksCfg> { HooksCfg* p; operator HooksCfg* () { return p; } HooksCfg* operator -> () { return p; } }; }
#endif



