#pragma once
#include <string>
#include <xstring>
#include <format>
#include <debugapi.h>

using std::string;
using std::format;
/**
 * @brief Some helper constructs
*/
namespace Utils
{
	extern string ExeName;
#ifdef _USRDLL
	extern string DllName;
#endif // _USRDLL



#define STRINGIZE(x) #x
#define STR(x) STRINGIZE(x)
#define __FILE_LINE__ __FILE__ "(" STR(__LINE__) ")"

#ifdef _DEBUG

#define WRITE_DEBUG_LOG(msg) Utils::WriteDebugLog(format("{}\t{}\n", msg, __FUNCTION__ "\t\t" __FILE_LINE__))

#define WRITE_DEBUG_LOG_DETAIL(msg, dtl) Utils::WriteDebugLog(format("{}\t{}\t{}\n", msg, __FUNCTION__ "\t\t" __FILE_LINE__, dtl))

#define WRITE_DEBUG_LOG_DURATION(msg, methodCall) \
{ \
	TDateTime before = Now(); \
	methodCall; \
	int duration = MilliSecondsBetween(before, Now()); \
	Utils::WriteDebugLog(format("{}\t{}\n"", msg, format("{}\t{}\t{}, __FUNCTION__, duration, __FILE_LINE__))); \
} \

#define WRITE_DEBUG_LOG_DETAIL_DURATION(msg, dtl, methodCall) \
{ \
	TDateTime before = Now(); \
	methodCall; \
	int duration = MilliSecondsBetween(before, Now()); \
	Utils::WriteDebugLog(format("{}\t{}\t{}\n", msg, format("{}\t{}\t{}", __FUNCTION__, duration, __FILE_LINE__)), dtl); \
} \

	void WriteDebugLog(string msg);


#else

#define WRITE_DEBUG_LOG(msg)
#define WRITE_DEBUG_LOG_DETAIL(msg, dtl)
#define WRITE_DEBUG_LOG_DURATION(msg, methodCall) { methodCall; }
#define WRITE_DEBUG_LOG_DETAIL_DURATION(msg, dtl, methodCall)  { methodCall; }

	inline void WriteDebugLog(string msg)
	{
		::OutputDebugStringA(msg.c_str());
	}

#endif // !_DEBUG
	/**
	 * dynamic format
	 */
	template <typename... Args>
	string dformat(std::string_view rt_fmt_str, Args&&... args) 
	{
		return std::vformat(rt_fmt_str, std::make_format_args(args...));
	}

	inline void ShowLastError(const string fmtStr)
	{
		LPSTR errorText = NULL;
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&errorText,
			0,
			NULL);
		const string errorStr = errorText;

		string msg = dformat(fmtStr, errorStr);
		MessageBoxA(NULL, msg.c_str(), "Error", MB_ICONERROR | MB_OK);
		LocalFree(errorText);
	}
}