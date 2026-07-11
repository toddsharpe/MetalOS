#pragma once

#include "user/MetalOS.h"

namespace Runtime
{
	ThreadEnvironmentBlock* GetTEB();
	ProcessEnvironmentBlock* GetPEB();
	Module* GetLoadedModule(const char* name);
	bool IsDebug();
}
