#include "pch.h"
#include "Config.h"
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
#include <document.h>
#include <writer.h>
#include <shellapi.h>
#include <Utils.h>
#include "WinPosWndConfig.h"
/**
 * @brief Read the general config
*/
void Config::readConfig()
{
	using namespace rapidjson;

	{
		RECT wa = { 0 };
		SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
		int waw = wa.right - wa.left, wah = wa.bottom - wa.top;
		int w = 600, h = 400;
		Rect.left = (waw - w) / 2;
		Rect.top = (wah - h) / 2;
		Rect.right = Rect.left + w;
		Rect.bottom = Rect.top + h;
	}

	char filePath[MAX_PATH];
	GetModuleFileNameA(NULL, filePath, MAX_PATH);
	string exeName = PathFindFileNameA(filePath);
	char buffer[4096] = { 0 };
	char configPath[MAX_PATH];
	ExpandEnvironmentStringsA(std::format(R"(%APPDATA%\wiNpoS\{}.jsonl)", exeName).c_str(), configPath, MAX_PATH);
	WRITE_DEBUG_LOG(format("Read config for {} from {}", exeName, configPath));

	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, configPath, "rb"); // non-Windows use "r"
	if (fp)
	{
		FileReadStream is(fp, buffer, sizeof(buffer));
		fclose(fp);

		Document d;
		d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(is);

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
 * @brief Save the general config
*/
void Config::writeConfig()
{
	char filePath[MAX_PATH];
	GetModuleFileNameA(NULL, filePath, MAX_PATH);
	string exeName = PathFindFileNameA(filePath);
	//wchar_t buffer[4096] = { 0 };
	char configPath[MAX_PATH];
	ExpandEnvironmentStringsA(std::format(R"(%APPDATA%\wiNpoS\{}.jsonl)", exeName).c_str(), configPath, MAX_PATH);
	WRITE_DEBUG_LOG(format("Write config for {} to {}", exeName, configPath));

	{
		wchar_t dir[MAX_PATH];
		ExpandEnvironmentStrings(LR"(%APPDATA%\wiNpoS)", dir, MAX_PATH);
		CreateDirectory(dir, NULL);
		{
			std::ofstream os(configPath);
			os << std::format(R"(
{{
	"left" :{},
	"top" : {},
	"right" : {},
	"bottom" : {}
}}
)",
Rect.left,
Rect.top,
Rect.right,
Rect.bottom) << std::endl;

		}
	}
}
/**
 * @brief open the cofig folder
*/
void Config::openFolder()
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
void Config::openWinPosConfig()
{
	string configPath = WinPosWndConfig::getConfigPath();
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

#pragma warning(pop)

