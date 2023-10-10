#include "pch.h"
#include "WinPosWndConfig.h"
#include <format>
#pragma warning(push)
#pragma warning( disable : 33010 26451 26495 26819 )
#include <filereadstream.h>
#include <filewritestream.h>
#include <prettywriter.h>
#include <document.h>
#include <writer.h>
#include <xlocbuf>
#include <codecvt>
#include <tuple>

namespace
{
	using MonitorInfos = std::vector<MONITORINFOEXA>;
	BOOL CALLBACK Monitorenumproc(HMONITOR hMon, HDC hDC, LPRECT pRECT, LPARAM lParam)
	{
		MonitorInfos& mInfos = *(MonitorInfos*)lParam;
		MONITORINFOEXA mi = { sizeof(MONITORINFOEXA) };
		GetMonitorInfoA(hMon, &mi);
		mInfos.push_back(mi);
		return TRUE;
	}

	bool operator == (const RECT& x, const RECT& y)
	{
		using std::make_tuple;
		return make_tuple(x.left, x.top, x.right, x.bottom) == make_tuple(y.left, y.top, y.right, y.bottom);
	}
}

void WinPosWndConfig::readConfig()
{
	using std::format;
	string configPath = getConfigPath();
	WRITE_DEBUG_LOG(format("Read config from {}", configPath));

	MonitorInfos monitorInfos;
	EnumDisplayMonitors(NULL, NULL, Monitorenumproc, (LPARAM)&monitorInfos);

	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, configPath.c_str(), "rb"); // non-Windows use "r"
	if (fp)
	{
		using namespace rapidjson;
		char buffer[4096*8] = { 0 };
		FileReadStream is(fp, buffer, sizeof(buffer));
		fclose(fp);

		Document d;
		d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(is);

		if (d.HasMember("PosWndConfig"))
		{
			Value& c = d["PosWndConfig"];
			if (c.HasMember("Scale"))
				_scale = c["Scale"].GetFloat();
			if (c.HasMember("CloseTimeout"))
				_closeTimeout = c["CloseTimeout"].GetUint();
			if (c.HasMember("Monitors"))
			{
				LONG xoffset = 0;
				for (const auto& m : c["Monitors"].GetArray())
				{
					if (m.HasMember("Index"))
					{
						uint32_t index = m["Index"].GetUint();
						if (index < monitorInfos.size())
						{
							auto& mi = monitorInfos[index];
							string deviceName;
							DISPLAY_DEVICEA dd = { sizeof(DISPLAY_DEVICEA) };
							if (EnumDisplayDevicesA(mi.szDevice, 0, &dd, 0))
								deviceName = dd.DeviceString;

							RECT previewRect = mi.rcWork;
							OffsetRect(&previewRect, -previewRect.left + xoffset, 0); // move left to 0, then to the right of the predecessor
							xoffset += (previewRect.right - previewRect.left) - GetSystemMetrics(SM_CXBORDER);
							MonitorConfig mc = { mi.rcWork, ScaleRect(previewRect, getScale()), mi.szDevice, deviceName, index };
							if (m.HasMember("Positions"))
							{
								static auto getRect = 
									[](Value& v)->RECT
									{
										RECT r = {
											v["Left"].GetInt(),
											v["Top"].GetInt(),
											v["Width"].GetInt(),
											v["Height"].GetInt(),
										};
										return r;
									};

								for (const auto& p : m["Positions"].GetArray())
								{
									PosPreviewConfig pc = {
										getRect((Value&)p["WndRect"]),
										p.HasMember("PrvRect") ? getRect((Value&)p["PrvRect"]) : RECT{0},
										p.HasMember("Name") ? p["Name"].GetString() : string(),
										lstrcmpA(p["Units"].GetString(), "%") == 0 ? Percent : Pixels,
									};
									if (pc.prvRect == RECT{ 0 })
										pc.prvRect = pc.units == Pixels ? ScaleRect(pc.wndRect, getScale()) : pc.wndRect;

									mc.previews.push_back(pc);
								}
							}
							_monitors.push_back(mc);
						}
					}
				}
			}

			return;
		}
	} 

	// something was not finished above. Use default.
	{
		// standard configuration
		_closeTimeout = 3500;
		_scale = 0.12f;
		LONG xoffset = 0;
		uint32_t index = 0;
		for (const auto& mi : monitorInfos)
		{
			string deviceName;
			DISPLAY_DEVICEA dd = { sizeof(DISPLAY_DEVICEA) };
			if (EnumDisplayDevicesA(mi.szDevice, 0, &dd, 0))
				deviceName = dd.DeviceString;

			RECT previewRect = mi.rcWork;
			OffsetRect(&previewRect, -previewRect.left + xoffset, 0); // move left to 0, then to the right of the predecessor
			xoffset += (previewRect.right - previewRect.left) - GetSystemMetrics(SM_CXBORDER);
			MonitorConfig mc = { mi.rcWork, ScaleRect(previewRect, getScale()), mi.szDevice, deviceName, index++ };
			mc.previews = { 
				{ {5,5,20,20},		{0}, "Top-Left", Percent },
				{ {25,5,20,20},	{0}, "Top-Right", Percent },
				{ {5,25,20,20},	{0}, "Bottom-Left", Percent },
				{ {25,25,20,20},	{0}, "Bottom-Right", Percent },
			};
			_monitors.push_back(mc);
		}
		writeConfig();
	}

}

void WinPosWndConfig::writeConfig()
{
	using namespace rapidjson;
	Document d;
	d.SetObject();
	auto& a = d.GetAllocator();

	Value c(kObjectType); // config
	c.AddMember("Scale", getScale(), a);
	c.AddMember("CloseTimeout", getCloseTimeout(), a);

	Value mcs(kArrayType); // monitor configs

	for (const auto& mc : _monitors)
	{
		Value m(kObjectType); // monitor config
		{
			Value v;
			v.SetString(mc.name.c_str(), a);
			m.AddMember("Device", v, a);
		}
		//mc["Device"].SetString(m.device.c_str(), a);
		{
			Value v;
			RECT mr = mc.monitorRect;
			v.SetString(format("{},{},{},{}", mr.left, mr.top, mr.right - mr.left, mr.bottom - mr.top).c_str(), a);
			m.AddMember("MonitorRect", v, a);
		}
		{
			Value v;
			v.SetUint(mc.index);
			m.AddMember("Index", v, a);
		}

		static auto getRectObject = 
			[&a](RECT r)->Value
			{
				Value v(kObjectType);
				v.AddMember("Left", (int)r.left, a);
				v.AddMember("Top", (int)r.top, a);
				v.AddMember("Width", (int)r.right, a);
				v.AddMember("Height", (int)r.bottom, a);
				return v;
			};

		Value pcs(kArrayType); // position configs
		for (auto& pc : mc.previews)
		{
			Value p(kObjectType); // position config
			{
				Value v = getRectObject(pc.wndRect);
				p.AddMember("WndRect", v, a);
			}
			{
				Value v = getRectObject(pc.prvRect);
				p.AddMember("PrvRect", v, a);
			}
			{
				Value v;
				v.SetString(StringRef(pc.name.c_str()), a);
				p.AddMember("Name", v, a);
			}
			{
				Value v;
				v.SetString(pc.units == Percent ? "%" : "px", a);
				p.AddMember("Units", v, a);
			}
			pcs.PushBack(p, a);
		}
		m.AddMember("Positions", pcs, a);

		mcs.PushBack(m, a);
	}

	c.AddMember("Monitors", mcs, a);

	d.AddMember("PosWndConfig", c, a);

	string configPath = getConfigPath();
	FILE* fp = nullptr;
	errno_t res = fopen_s(&fp, configPath.c_str(), "wb"); // non-Windows use "w"
	assert(fp);

	char buffer[4096];
	FileWriteStream os(fp, buffer, sizeof(buffer));

	PrettyWriter<FileWriteStream> writer(os);
	d.Accept(writer);

	fclose(fp);
	WRITE_DEBUG_LOG(format("Write config to {}", configPath));
}

string WinPosWndConfig::getConfigPath(const string& configName /*= "WndConfig"*/)
{
	using std::format;
	char configPath[MAX_PATH];
	ExpandEnvironmentStringsA(format(R"(%APPDATA%\wiNpoS\{}.jsonc)", configName).c_str(), configPath, MAX_PATH);
	return configPath;
}
