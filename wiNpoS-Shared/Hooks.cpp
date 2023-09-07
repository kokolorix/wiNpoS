#include "pch.h"
#include "Hooks.h"
#include <cassert>

void Hooks::attach()
{
#ifdef Win32
	_hModule = LoadLibrary(L"wiNpoS-Hook32.dll");
#else
	_hModule = LoadLibrary(L"wiNpoS-Hook64.dll");
#endif // Win32

}

void Hooks::detach()
{
	assert(FreeLibrary(_hModule));
}
