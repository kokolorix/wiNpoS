#include "pch.h"
#include "DebugNew.h"
#include "Utils.h"
#include <fstream>
#include <iosfwd>
#include <Shlwapi.h>
#include <chrono>
#include <format>
#include <vector>
#include <winuser.h>
#include <winver.h>
#include <strsafe.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "version.lib")

using std::ofstream;
using std::wstring;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
//#ifdef _USRDLL
extern HINSTANCE hInstance;   // current instance
//#endif // _USRDLL

namespace
{
	string initLogFile()
	{
		char buffer[MAX_PATH] = { 0 };
#ifdef _USRDLL
		GetModuleFileNameA(hInstance, buffer, MAX_PATH);
#else
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
#endif // _USRDLL
		string moduleName = PathFindFileNameA(buffer);



		char logPath[MAX_PATH];
		ExpandEnvironmentStringsA(format(R"(%APPDATA%\wiNpoS\{}-debug1.log)", moduleName).c_str(), logPath, MAX_PATH);

		ofstream fs(logPath);
		fs << "Zeit\tProcessName\tProzessId\tThreadId\tMessage\tFunction\tLogSource\tDetail" << std::endl;

		return logPath;
	}


	string initExeName()
	{
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		string  exeName = PathFindFileNameA(buffer);
		return exeName;
	}
	string initBinDir()
	{
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(hInstance, buffer, MAX_PATH);
		PathRemoveFileSpecA(buffer);
		return buffer;
	}
	/**
	* Get file infos from version resources
	* Possible Values from infop are:
	* Comments 				InternalName 			ProductName
	* CompanyName 			LegalCopyright 		ProductVersion
	* FileDescription 	LegalTrademarks 		PrivateBuild
	* FileVersion 			OriginalFilename 		SpecialBuild
	*/
	string initVersionInfo(string info)
	{
		DWORD laenge = 0;
		laenge = GetFileVersionInfoSizeA(Utils::ExeName.c_str(), &laenge);

		if (laenge > 0)
		{
			auto buf = std::make_unique<char[]>(laenge);
			GetFileVersionInfoA(Utils::ExeName.c_str(), 0, laenge, buf.get());

			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			UINT len;
			VerQueryValueA(buf.get(), R"(\VarFileInfo\Translation)", reinterpret_cast<void**>(&lpTranslate), &len);
			string subblock = dformat(R"(\StringFileInfo\{:04x}{:04x}\{})", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage, info);
			char* value;
			if (VerQueryValueA(buf.get(), subblock.c_str(), reinterpret_cast<void**>(&value), &len))
				return value;
		}
		return "";
	}

#ifdef _USRDLL
	string initDllName()
	{
		char buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(hInstance, buffer, MAX_PATH);

		string  dllName = PathFindFileNameA(buffer);
		return dllName;
}
#endif // _USRDLL
}
string Utils::ExeName = initExeName();
string Utils::BinDir = initBinDir();
string Utils::ProductVersion = initVersionInfo("ProductVersion");
string Utils::ProductName = initVersionInfo("ProductName");
 
#ifdef _USRDLL
string Utils::DllName = initDllName();
#endif // _USRDLL

#ifdef _DEBUG

void Utils::WriteDebugLog(std::string msg)
{
	::OutputDebugStringA(msg.c_str());

	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);
	//auto now = system_clock::now();
	//auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

	//string timestamp = std::vformat("{:%T}.{:03}", std::make_format_args(system_clock::to_time_t(now), ms % 1000));
	std::ostringstream os;
	os << std::setfill('0') << std::setw(2) << currentTime.wHour << ":";
	os << std::setfill('0') << std::setw(2) << currentTime.wMinute << ":";
	os << std::setfill('0') << std::setw(2) << currentTime.wSecond << ".";
	os << std::setfill('0') << std::setw(3) << currentTime.wMilliseconds;
	string timestamp = os.str();

	char filePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, filePath, MAX_PATH);
	string exeName = PathFindFileNameA(filePath);

	string logEntry = format("{}\t{}\t{}\t{}\t{}", timestamp, exeName, GetCurrentProcessId(), GetCurrentThreadId(), msg);

	static string logPath = initLogFile();
	ofstream fs(logPath, std::ios::app);
	fs << logEntry << std::endl;
}

#endif
/**
 * @brief private stuff to determine the top level windows
*/
namespace
{
	using WndVector = std::vector<HWND>;
	BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
	{
		WndVector& hWnds = *reinterpret_cast<WndVector*>(lParam);
		hWnds.push_back(hwnd);
		return TRUE;
	}

	WndVector GetDesktopWnds()
	{
		WndVector hWnds;
		auto threadId = GetCurrentThreadId();
		auto hDesktop = GetThreadDesktop(threadId);

		auto res = EnumDesktopWindows(hDesktop, EnumWindowsProc, (LPARAM)&hWnds);

		return hWnds;	//EnumDesktopWindows()
	}
}
/*
 * @brief get the top level window for the given process- and thread-id
 * @param threadId 
 * @param processId 
 * @return 
*/
Utils::MainWndRes Utils::GetMainWnd(DWORD threadId /*= 0*/, DWORD processId /*= 0*/)
{
	auto wnds = GetDesktopWnds();

	processId = processId ? processId : GetCurrentProcessId();

	for (HWND hWnd : wnds)
	{
		DWORD wndProcessId = NULL;
		auto wndThreadId = GetWindowThreadProcessId(hWnd, &wndProcessId);
		if (processId == wndProcessId && (threadId == 0 || wndThreadId == threadId))
		{
			HWND hMainWnd = GetAncestor(hWnd, GA_ROOTOWNER);
			MainWndRes res = { hMainWnd, wndThreadId };
			return res;
		}
	}

	MainWndRes res = { 0 };
	return res;
}

namespace
{
	using DesktopNameVector = std::vector<string>;
	BOOL CALLBACK EnumDesktopProcA(_In_ LPSTR lpszDesktop, _In_ LPARAM lParam)
	{
		DesktopNameVector& desktopNames = *reinterpret_cast<DesktopNameVector*>(lParam);
		desktopNames.push_back(string(lpszDesktop));
		return TRUE;
	}
}

Utils::MainWindResVector Utils::GetMainWnds(DWORD threadId /*= 0*/, DWORD processId /*= 0*/)
{
	DesktopNameVector desktopNames;
	EnumDesktopsA(nullptr, EnumDesktopProcA, (LPARAM)&desktopNames);

	MainWindResVector mainWnds;
	for (const string dn : desktopNames)
	{
		HDESK hDesk = OpenDesktopA(dn.c_str(), 0, FALSE, READ_CONTROL);
		WndVector hWnds;
		auto res = EnumDesktopWindows(hDesk, EnumWindowsProc, (LPARAM)&hWnds);
		CloseDesktop(hDesk);
		for (HWND hWnd : hWnds)
		{
			DWORD wndProcessId = NULL;
			auto wndThreadId = GetWindowThreadProcessId(hWnd, &wndProcessId);
			if (processId == wndProcessId && (threadId == 0 || wndThreadId == threadId))
			{
				HWND hMainWnd = GetAncestor(hWnd, GA_ROOTOWNER);
				mainWnds.push_back({ hMainWnd, wndThreadId });
			}
		}
	}
	return mainWnds;
}

RECT Utils::ScaleRect(const RECT& in, FLOAT f)
{
	RECT r = { 0 };

	r.right = static_cast<LONG>(((FLOAT)((in.right - in.left) + in.left)) * f);
	r.bottom = static_cast<LONG>(((FLOAT)((in.bottom - in.top) + in.top)) * f);
	r.left = static_cast<LONG>(((FLOAT)in.left) * f);
	r.top = static_cast<LONG>(((FLOAT)in.top) * f);

	return r;
}

