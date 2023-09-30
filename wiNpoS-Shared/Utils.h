#pragma once
#include <string>
#include <xstring>
#include <format>
#include <debugapi.h>
#include <vector>

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
	template <typename... Args>
	std::wstring dwformat(std::wstring_view rt_fmt_str, Args&&... args) 
	{
		return std::vformat(rt_fmt_str, std::make_wformat_args(args...));
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

	template<typename T, typename Tx>
	bool is_one_of(T t, Tx&& p1)
	{
		return t == p1;
	}

	template<typename T, typename T1, typename... Tx>
	bool is_one_of(T t, T1&& p1, Tx&&... px) {
		return is_one_of(t, p1) || is_one_of(t, px...);
	}

	template<typename T, typename Tx>
	bool is_each_of(T t, Tx&& p1)
	{
		return t == p1;
	}

	template<typename T, typename T1, typename... Tx>
	bool is_each_of(T t, T1&& p1, Tx&&... px) {
		return is_each_of(t, p1) && is_each_of(t, px...);
	}

	template<typename T = uint32_t, typename M = T, typename F = T>	
	inline bool check_bits(M bitmask, F val) 
	{ 
		return ((T)val & (T)bitmask) == bitmask; 
	}
	template<typename T = uint32_t, typename M = T, typename F = T>	
	inline bool check_one_bit(M bitmask, F val) 
	{ 
		return (((T)val) & ((T)bitmask)) != 0; 
	}
	template<typename T = uint32_t, typename M = T, typename F = T>	
	inline M set_bits(M bitmask, F val = 0) 
	{
		return ((T)val |= (T)bitmask); 
	}
	template<typename T = uint32_t, typename M = T, typename F = T>	
	inline M clear_bits(M bitmask, F val = 0)
	{ 
		return (T)val &= ~(T)bitmask;
	}
}
using Utils::dformat;
using Utils::dwformat;

using Utils::is_one_of;
using Utils::is_each_of;

using Utils::check_bits;
using Utils::check_one_bit;
using Utils::set_bits;
using Utils::clear_bits;

namespace Utils
{
	struct MainWndRes
	{
		HWND hMainWnd;
		uint32_t wndThreadId;

		operator HWND () const { return hMainWnd; }
		operator bool() const { return hMainWnd != NULL; }
	};

	MainWndRes GetMainWnd(DWORD threadId = 0, DWORD processId = 0);

	using MainWindResVector = std::vector<MainWndRes>;
	MainWindResVector GetMainWnds(DWORD threadId = 0, DWORD processId = 0);
}
using Utils::MainWindResVector;
using Utils::GetMainWnds;
using Utils::MainWndRes;
using Utils::GetMainWnd;