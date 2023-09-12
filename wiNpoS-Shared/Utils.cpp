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
		fs << "Zeit\tProzessId\tThreadId\tMessage\tFunction\tLogSource\tDetail" << std::endl;

		return logPath;
	}
}

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

	string logEntry = format("{}\t{}\t{}\t{}", timestamp, ::GetCurrentProcessId(), ::GetCurrentThreadId(), msg);

	string logPath = initLogFile();
	ofstream fs(logPath, std::ios::app);
	fs << logEntry << std::endl;
}

#endif


