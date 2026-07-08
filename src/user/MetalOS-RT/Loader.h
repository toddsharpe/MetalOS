#pragma once

#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.Types.h"

namespace Loader
{
	typedef void (*CrtInitializer)();
	void CrtInit(const void* const imageBase);
}

extern "C"
{
	HModule LoadLibrary(char* lpLibFileName);
	uintptr_t GetProcAddress(HModule hModule, const char* lpProcName);
}
