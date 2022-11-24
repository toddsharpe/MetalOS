#pragma once

#include <shared/MetalOS.Types.h>

class Runtime
{
public:
	static ThreadEnvironmentBlock* GetTEB();
	static ProcessEnvironmentBlock* GetPEB();
	static Module* GetLoadedModule(const char* name);
};
