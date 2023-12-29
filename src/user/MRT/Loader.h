#pragma once

#include <MetalOS.h>

class Loader
{
public:
	static Handle LoadLibrary(const char* lpLibFileName);
	static void CrtInit(Handle moduleBase);
};
