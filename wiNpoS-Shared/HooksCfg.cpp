#include "pch.h"
#include <format>
#include <cstdio>
#include <processenv.h>
#include <locale>
#include <codecvt>
#include <fstream>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#pragma warning(push)
#pragma warning( disable : 33010 26451 26495 26819 )
#include <filereadstream.h>
#include <filewritestream.h>
#include <prettywriter.h>
#include <document.h>
#include <writer.h>
#include <shellapi.h>
#include <Utils.h>

#include "DebugNew.h"
#include "HooksCfg.h"
using namespace rapidjson;

#include "WinPosWndCfg.h"

/**
 * @brief read a JSON config from given path
 * @param configPath 
 * @return 
*/
DocumentPtr HooksCfg::readConfigDocument(const string& configPath)
{
	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, configPath.c_str(), "rb"); // non-Windows use "r"
	if (fp)
	{
		static const size_t BUFFER_SIZE = 4096 * 8;
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(BUFFER_SIZE);

		FileReadStream is(fp, buffer.get(), BUFFER_SIZE);
		fclose(fp);

		auto d_ptr = std::shared_ptr<Document>(new Document);
		d_ptr->ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(is);
		return d_ptr;
	}

	return DocumentPtr();
}
/**
 * @brief save the JSON document to the given path
 * @param configPath 
 * @param d 
*/
void HooksCfg::writeConfigDocument(Document& d, const string& configPath)
{
	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, configPath.c_str(), "wb"); // non-Windows use "w"
	AssertTrue(fp, dformat("File {} should be open here", configPath));

	static const size_t BUFFER_SIZE = 4096 * 8;
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(BUFFER_SIZE);
	FileWriteStream os(fp, buffer.get(), BUFFER_SIZE);

	PrettyWriter<FileWriteStream> writer(os);
	d.Accept(writer);

	fclose(fp);
	WRITE_DEBUG_LOG(format("Write config to {}", configPath));
}

/**
 * @brief read the config for this app
*/
void HooksCfg::readAppConfig()
{
	{ // default rect
		RECT wa = { 0 };
		SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
		int waw = wa.right - wa.left, wah = wa.bottom - wa.top;
		int w = 600, h = 400;
		Rect.left = (waw - w) / 2;
		Rect.top = (wah - h) / 2;
		Rect.right = Rect.left + w;
		Rect.bottom = Rect.top + h;
	}

	if(auto d_ptr = readConfigDocument(getAppConfigPath()))
	{
		Document& d = *d_ptr;

		if (d.HasMember("left"))
			Rect.left = d["left"].GetInt();
		if (d.HasMember("top"))
			Rect.top = d["top"].GetInt();
		if (d.HasMember("right"))
			Rect.right = d["right"].GetInt();
		if (d.HasMember("bottom"))
			Rect.bottom = d["bottom"].GetInt();
	}
}
/**
 * @brief read the global config for all apps
*/
void HooksCfg::readConfig()
{
	if(auto d_ptr = readConfigDocument(getConfigPath()))
	{
		Document& d = *d_ptr;
		_disabled = d.HasMember("disabled") ? d["disabled"].GetBool() : false;
	}
}
/**
 * @brief save the config for this app
*/
void HooksCfg::writeAppConfig()
{
	string configDir = getConfigDir();
	CreateDirectoryA(configDir.c_str(), NULL);

	Document d;
	d.SetObject();
	auto& a = d.GetAllocator();

	d.AddMember("left", (int)Rect.left, a);
	d.AddMember("top", (int)Rect.top, a);
	d.AddMember("right", (int)Rect.right, a);
	d.AddMember("bottom", (int)Rect.bottom, a);

	string configPath = getAppConfigPath();
	writeConfigDocument(d, configPath);
}
/**
 * @brief save global config for all apps
*/
void HooksCfg::writeConfig()
{
	Document d;
	d.SetObject();
	auto& a = d.GetAllocator();
	d.AddMember("disabled", _disabled, a);

	string configPath = getConfigPath();
	writeConfigDocument(d, configPath);
}
/**
 * @brief open the config folder
*/
void HooksCfg::openFolder()
{
	wchar_t dir[MAX_PATH];
	ExpandEnvironmentStrings(LR"(%APPDATA%\wiNpoS)", dir, MAX_PATH);

	HINSTANCE hFolder = ShellExecute(
		NULL,
		L"open",
		dir,
		NULL,
		dir,
		SW_SHOW
	);

}
/**
 * @brief open the wiNpoS config in editor
*/
void HooksCfg::openWinPosConfig()
{
	string configPath = WinPosWndCfg::getConfigPath();
	char configDir[MAX_PATH];
	ExpandEnvironmentStringsA(R"(%APPDATA%\wiNpoS)", configDir, MAX_PATH);
	HINSTANCE hConfig = ShellExecuteA(
		NULL,
		"open",
		configPath.c_str(),
		NULL,
		configDir,
		SW_SHOW
	);

}
/**
 * @brief stores disabled in config
*/
void HooksCfg::disable()
{
	_disabled = true;
	writeConfig();
}
/**
 * @brief remove disabled from config
*/
void HooksCfg::enable()
{
	_disabled = false;
	writeConfig();
}
/**
 * @brief check if someone has disabled stored in config
 * @return 
*/
bool HooksCfg::isDisabled()
{
	readConfig();
	return _disabled;
}
/**
 * @brief the path to the config file for this app
 * @return 
*/
string HooksCfg::getAppConfigPath()
{
	using std::format;
	char filePath[MAX_PATH];
	GetModuleFileNameA(NULL, filePath, MAX_PATH);
	string exeName = PathFindFileNameA(filePath);
	return format(R"({}\{}.jsonl)", getConfigDir(), exeName);
}
/**
 * @brief the path to the global config, for all apps
 * @return 
*/
string HooksCfg::getConfigPath()
{
	using std::format;
	return format(R"({}\wiNpoS.jsonl)", getConfigDir());
}

/**
 * @brief the directory, where store the configs and logs
 * @return 
*/
string HooksCfg::getConfigDir()
{
	char configDir[MAX_PATH];
	ExpandEnvironmentStringsA(R"(%APPDATA%\wiNpoS)", configDir, MAX_PATH);
	return configDir;
}

#pragma warning(pop)

