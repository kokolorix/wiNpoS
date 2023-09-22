#include "pch.h"
#include "Utils.h"
#include <fstream>
#include <iosfwd>
#include <Shlwapi.h>
#include <chrono>
#include <format>
#include <vector>
#pragma comment(lib, "Shlwapi.lib")

using std::ofstream;
using std::wstring;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
#ifdef _USRDLL
extern HINSTANCE hInstance;   // current instance
#endif // _USRDLL

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

	BOOL CALLBACK EnumWindowsProc(HWND   hWnd, LPARAM lParam)
	{
		WndVector& hWnds = *reinterpret_cast<WndVector*>(lParam);
		hWnds.push_back(hWnd);
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
Utils::GetMainWndRes Utils::GetMainWnd(DWORD threadId /*= 0*/, DWORD processId /*= 0*/)
{
	auto wnds = GetDesktopWnds();

	processId = processId ? processId : GetCurrentProcessId();

	for (HWND hWnd : wnds)
	{
		DWORD wndProcessId = NULL;
		auto wndThreadId = GetWindowThreadProcessId(hWnd, &wndProcessId);
		if (processId == wndProcessId && (threadId == 0 || wndThreadId == threadId))
		{
			HWND hMainWnd = GetAncestor(hWnd, GA_ROOT);
			GetMainWndRes res = { hMainWnd, wndThreadId };
			return res;
		}
	}

	GetMainWndRes res = { 0 };
	return res;
}


