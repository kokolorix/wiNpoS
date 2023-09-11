#include "pch.h"
#include "Utils.h"

void Utils::WriteDebugLog(std::string msg)
{
	::OutputDebugStringA(msg.c_str());
}


