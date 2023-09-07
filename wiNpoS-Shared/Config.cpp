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
	char* name = PathFindFileNameA(filePath);
	char buffer[4096] = { 0 };
	char path[MAX_PATH];
	ExpandEnvironmentStringsA(std::format(R"(%APPDATA%\wiNpoS\{}.jsonc)", name).c_str(), path, MAX_PATH);

	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, path, "rb"); // non-Windows use "r"
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

void Config::writeConfig()
{
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(NULL, filePath, MAX_PATH);
	wchar_t* name = PathFindFileName(filePath);
	wchar_t buffer[4096] = { 0 };
	wchar_t path[MAX_PATH];
	ExpandEnvironmentStrings(std::format(LR"(%APPDATA%\wiNpoS\{}.jsonc)", name).c_str(), path, MAX_PATH);

	{
		wchar_t dir[MAX_PATH];
		ExpandEnvironmentStrings(LR"(%APPDATA%\wiNpoS)", dir, MAX_PATH);
		CreateDirectory(dir, NULL);
		{
			std::ofstream os(path);
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
#pragma warning(pop)

