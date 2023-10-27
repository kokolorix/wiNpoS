#pragma once
#include <wtypes.h>
#include <string>
#include "Utils.h"

class Config
{
public:
	RECT Rect = { 0 };

public:
	void readConfig();
	void writeConfig();

	void openFolder();
	void openWinPosConfig();

private:
};

using ConfigPtr = std::unique_ptr<Config>;
#ifdef _DOXYGEN_RUNNING
namespace std { template<> class unique_ptr<Config> { Config* p; operator Config* () { return p; } Config* operator -> () { return p; } }; }
#endif



