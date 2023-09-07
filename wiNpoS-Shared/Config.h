#pragma once
#include <wtypes.h>
#include <string>

class Config
{
public:
	RECT Rect = { 0 };

public:
	void readConfig();
	void writeConfig();

private:
};

